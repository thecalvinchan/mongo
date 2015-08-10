/**
 * Copyright (C) 2015 MongoDB Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the copyright holders give permission to link the
 * code of portions of this program with the OpenSSL library under certain
 * conditions as described in each individual source file and distribute
 * linked combinations including the program with the OpenSSL library. You
 * must comply with the GNU Affero General Public License in all respects
 * for all of the code used other than as permitted herein. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you do not
 * wish to do so, delete this exception statement from your version. If you
 * delete this exception statement from all source files in the program,
 * then also delete it in the license file.
 */

#include "mongo/platform/basic.h"
#include "mongo/base/checked_cast.h"
#include "mongo/scripting/tinyjs/comparison_operator.h"
#include "mongo/scripting/tinyjs/object_accessor_operator.h"

namespace mongo {
namespace tinyjs {

const bool ComparisonOperator::isOptimized() const {
    return _optimized;
}

bool ComparisonOperator::optimizable (bool optimize, AndMatchExpression* root) {
    Value returnValueDummy = Value();
    bool leftChildOptimizable = this->getLeftChild()->optimizable(optimize, root);
    bool rightChildOptimizable = this->getRightChild()->optimizable(optimize, root);
    if (!(leftChildOptimizable && rightChildOptimizable)) {
        //only zero or one branch contains object accessor
        if (optimize) {
            _optimized = true;
            // optimize query here
            if (!(leftChildOptimizable || rightChildOptimizable)) {
            //if (!(leftChildOptimizable && rightChildOptimizable)) {
                // NOTE: THIS WAS CAUSING IT TO FAIL AHHHH
                // NOTE: BOOLEAN ALGEBRA
                // case where both sides are constant ]
                // TODO
            } else if (leftChildOptimizable) {
                // case where left child contains an object access and right child doesn't
                // TODO: for now, just assume that left child is one simple object access
                BSONObjBuilder* builder = new BSONObjBuilder();
                Value v = this->getRightChild()->evaluate(nullptr, returnValueDummy);
                std::string key = "constant";
                StringData keyStringData = StringData(key);
                v.addToBsonObj(builder, keyStringData);
                BSONObj* object = new BSONObj(builder->obj());
                std::string fieldName = (checked_cast<ObjectAccessorOperator*>(this->getLeftChild()))->getFullField();
                int rootObjIndex = fieldName.find_first_of('.');
                std::string* fieldPathString = new std::string(fieldName.substr(rootObjIndex + 1));
                std::unique_ptr<ComparisonMatchExpression> eq;
                switch (this->getType()) {
                    case TokenType::kGreaterThan: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new GTMatchExpression());
                        break;
                    }
                    case TokenType::kGreaterThanEquals: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new GTEMatchExpression());
                        break;
                    }
                    case TokenType::kLessThan: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new LTMatchExpression());
                        break;
                    }
                    case TokenType::kLessThanEquals: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new LTEMatchExpression());
                        break;
                    }
                    case TokenType::kTripleEquals: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new EqualityMatchExpression());
                        break;
                    }
                    default: {
                        return false;
                        break;
                    }
                }
                Status s = eq->init(*fieldPathString, object->getField(keyStringData));
                if (!s.isOK())
                    return false;

                root->add(eq.release());
            } else {
                // case where right child contains an object access and left child doesn't
                // TODO: for now, just assume that right child is one simple object access
                BSONObjBuilder* builder = new BSONObjBuilder();
                Value v = this->getLeftChild()->evaluate(nullptr, returnValueDummy);
                std::string key = "constant";
                StringData keyStringData = StringData(key);
                v.addToBsonObj(builder, keyStringData);
                BSONObj* object = new BSONObj(builder->obj());
                std::string fieldName = (checked_cast<ObjectAccessorOperator*>(this->getRightChild()))->getFullField();
                int rootObjIndex = fieldName.find_first_of('.');
                std::string* fieldPathString = new std::string(fieldName.substr(rootObjIndex + 1));
                std::unique_ptr<ComparisonMatchExpression> eq;
                switch (this->getType()) {
                    case TokenType::kGreaterThan: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new LTMatchExpression());
                        break;
                    }
                    case TokenType::kGreaterThanEquals: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new LTEMatchExpression());
                        break;
                    }
                    case TokenType::kLessThan: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new GTMatchExpression());
                        break;
                    }
                    case TokenType::kLessThanEquals: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new GTEMatchExpression());
                        break;
                    }
                    case TokenType::kTripleEquals: {
                        eq = std::unique_ptr<ComparisonMatchExpression> (new EqualityMatchExpression());
                        break;
                    }
                    default: {
                        return false;
                        break;
                    }
                }
                Status s = eq->init(*fieldPathString, object->getField(keyStringData));
                if (!s.isOK())
                    return false;

                root->add(eq.release());
            }
        }
    }
    return leftChildOptimizable || rightChildOptimizable;
}

}  // namespace tinyjs
}  // namespace mongo
