/**
 *    Copyright (C) 2015 MongoDB Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kQuery

#include "mongo/platform/basic.h"

#include "mongo/db/exec/sort_key_generator.h"

#include <vector>

#include "mongo/db/catalog/collection.h"
#include "mongo/db/exec/working_set.h"
#include "mongo/db/exec/working_set_computed_data.h"
#include "mongo/db/query/query_planner.h"
#include "mongo/util/log.h"

namespace mongo {

SortKeyGenerator::SortKeyGenerator(const Collection* collection,
                                   const BSONObj& sortSpec,
                                   const BSONObj& queryObj) {
    _collection = collection;
    _hasBounds = false;
    _sortHasMeta = false;
    _rawSortSpec = sortSpec;

    // 'sortSpec' can be a mix of $meta and index key expressions.  We pick it apart so that
    // we only generate Btree keys for the index key expressions.

    // The Btree key fields go in here.  We pass this fake index key pattern to the Btree
    // key generator below as part of generating sort keys for the docs.
    BSONObjBuilder btreeBob;

    // The pattern we use to woCompare keys.  Each field in 'sortSpec' will go in here with
    // a value of 1 or -1.  The Btree key fields are verbatim, meta fields have a default.
    BSONObjBuilder comparatorBob;

    BSONObjIterator it(sortSpec);
    while (it.more()) {
        BSONElement elt = it.next();
        if (elt.isNumber()) {
            // Btree key.  elt (should be) foo: 1 or foo: -1.
            comparatorBob.append(elt);
            btreeBob.append(elt);
        } else if (LiteParsedQuery::isTextScoreMeta(elt)) {
            // Sort text score decreasing by default.  Field name doesn't matter but we choose
            // something that a user shouldn't ever have.
            comparatorBob.append("$metaTextScore", -1);
            _sortHasMeta = true;
        } else {
            // Sort spec. should have been validated before here.
            verify(false);
        }
    }

    // Our pattern for woComparing keys.
    _comparatorObj = comparatorBob.obj();

    // The fake index key pattern used to generate Btree keys.
    _btreeObj = btreeBob.obj();

    // If we're just sorting by meta, don't bother with all the key stuff.
    if (_btreeObj.isEmpty()) {
        return;
    }

    // We'll need to treat arrays as if we were to create an index over them. that is,
    // we may need to unnest the first level and consider each array element to decide
    // the sort order.
    std::vector<const char*> fieldNames;
    std::vector<BSONElement> fixed;
    BSONObjIterator btreeIt(_btreeObj);
    while (btreeIt.more()) {
        BSONElement patternElt = btreeIt.next();
        fieldNames.push_back(patternElt.fieldName());
        fixed.push_back(BSONElement());
    }

    _keyGen.reset(new BtreeKeyGeneratorV1(fieldNames, fixed, false /* not sparse */));

    // The bounds checker only works on the Btree part of the sort key.
    getBoundsForSort(queryObj, _btreeObj);

    if (_hasBounds) {
        _boundsChecker.reset(new IndexBoundsChecker(&_bounds, _btreeObj, 1 /* == order */));
    }
}

Status SortKeyGenerator::getSortKey(const WorkingSetMember& member, BSONObj* objOut) const {
    BSONObj btreeKeyToUse;

    Status btreeStatus = getBtreeKey(member.obj.value(), &btreeKeyToUse);
    if (!btreeStatus.isOK()) {
        return btreeStatus;
    }

    if (!_sortHasMeta) {
        *objOut = btreeKeyToUse;
        return Status::OK();
    }

    BSONObjBuilder mergedKeyBob;

    // Merge metadata into the key.
    BSONObjIterator it(_rawSortSpec);
    BSONObjIterator btreeIt(btreeKeyToUse);
    while (it.more()) {
        BSONElement elt = it.next();
        if (elt.isNumber()) {
            // Merge btree key elt.
            mergedKeyBob.append(btreeIt.next());
        } else if (LiteParsedQuery::isTextScoreMeta(elt)) {
            // Add text score metadata
            double score = 0.0;
            if (member.hasComputed(WSM_COMPUTED_TEXT_SCORE)) {
                const TextScoreComputedData* scoreData = static_cast<const TextScoreComputedData*>(
                    member.getComputed(WSM_COMPUTED_TEXT_SCORE));
                score = scoreData->getScore();
            }
            mergedKeyBob.append("$metaTextScore", score);
        }
    }

    *objOut = mergedKeyBob.obj();
    return Status::OK();
}

Status SortKeyGenerator::getBtreeKey(const BSONObj& memberObj, BSONObj* objOut) const {
    // Not sorting by anything in the key, just bail out early.
    if (_btreeObj.isEmpty()) {
        *objOut = BSONObj();
        return Status::OK();
    }

    // We will sort '_data' in the same order an index over '_pattern' would have.  This is
    // tricky.  Consider the sort pattern {a:1} and the document {a:[1, 10]}. We have
    // potentially two keys we could use to sort on. Here we extract these keys.
    BSONObjCmp patternCmp(_btreeObj);
    BSONObjSet keys(patternCmp);

    try {
        _keyGen->getKeys(memberObj, &keys);
    } catch (const UserException& e) {
        // Probably a parallel array.
        if (BtreeKeyGenerator::ParallelArraysCode == e.getCode()) {
            return Status(ErrorCodes::BadValue, "cannot sort with keys that are parallel arrays");
        } else {
            return e.toStatus();
        }
    } catch (...) {
        return Status(ErrorCodes::InternalError, "unknown error during sort key generation");
    }

    // Key generator isn't sparse so we should at least get an all-null key.
    invariant(!keys.empty());

    // No bounds?  No problem!  Use the first key.
    if (!_hasBounds) {
        // Note that we sort 'keys' according to the pattern '_btreeObj'.
        *objOut = *keys.begin();
        return Status::OK();
    }

    // To decide which key to use in sorting, we must consider not only the sort pattern but
    // the query.  Assume we have the query {a: {$gte: 5}} and a document {a:1}.  That
    // document wouldn't match the query.  As such, the key '1' in an array {a: [1, 10]}
    // should not be considered as being part of the result set and thus that array cannot
    // sort using the key '1'.  To ensure that the keys we sort by are valid w.r.t. the
    // query we use a bounds checker.
    verify(NULL != _boundsChecker.get());
    for (BSONObjSet::const_iterator it = keys.begin(); it != keys.end(); ++it) {
        if (_boundsChecker->isValidKey(*it)) {
            *objOut = *it;
            return Status::OK();
        }
    }

    // No key is in our bounds.
    // TODO: will this ever happen?  don't think it should.
    *objOut = *keys.begin();
    return Status::OK();
}

void SortKeyGenerator::getBoundsForSort(const BSONObj& queryObj, const BSONObj& sortObj) {
    QueryPlannerParams params;
    params.options = QueryPlannerParams::NO_TABLE_SCAN;

    // We're creating a "virtual index" with key pattern equal to the sort order.
    IndexEntry sortOrder(
        sortObj, IndexNames::BTREE, true, false, false, "doesnt_matter", NULL, BSONObj());
    params.indices.push_back(sortOrder);

    auto statusWithQueryForSort =
        CanonicalQuery::canonicalize(NamespaceString("fake.ns"), queryObj, WhereCallbackNoop());
    verify(statusWithQueryForSort.isOK());
    std::unique_ptr<CanonicalQuery> queryForSort = std::move(statusWithQueryForSort.getValue());

    std::vector<QuerySolution*> solns;
    LOG(5) << "Sort key generation: Planning to obtain bounds for sort.";
    QueryPlanner::plan(*queryForSort, params, &solns);

    // TODO: are there ever > 1 solns?  If so, do we look for a specific soln?
    if (1 == solns.size()) {
        IndexScanNode* ixScan = NULL;
        QuerySolutionNode* rootNode = solns[0]->root.get();

        if (rootNode->getType() == STAGE_FETCH) {
            FetchNode* fetchNode = static_cast<FetchNode*>(rootNode);
            if (fetchNode->children[0]->getType() != STAGE_IXSCAN) {
                delete solns[0];
                // No bounds.
                return;
            }
            ixScan = static_cast<IndexScanNode*>(fetchNode->children[0]);
        } else if (rootNode->getType() == STAGE_IXSCAN) {
            ixScan = static_cast<IndexScanNode*>(rootNode);
        }

        if (ixScan) {
            _bounds.fields.swap(ixScan->bounds.fields);
            _hasBounds = true;
        }
    }

    for (size_t i = 0; i < solns.size(); ++i) {
        delete solns[i];
    }
}

}  // namespace mongo
