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
#include "mongo/bson/bsontypes.h"
#include "mongo/base/checked_cast.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/scripting/tinyjs/binary_operator.h"
#include "mongo/scripting/tinyjs/object_accessor_operator.h"


namespace mongo {
namespace tinyjs {

/*ObjectAccessorOperator::ObjectAccessorOperator(TokenType t, std::string objectPathString) : BinaryOperator(t) {
    _objectPathString = objectPathString;
}*/

ObjectAccessorOperator::ObjectAccessorOperator(TokenType t) : BinaryOperator(t) {}

const Value ObjectAccessorOperator::evaluate(Scope* scope) const {
    std::string objectPathString = ObjectAccessorOperator::generateNestedField(this, scope);
    int rootObjIndex = objectPathString.find_first_of('.');
    std::string fieldPathString = objectPathString.substr(rootObjIndex + 1),
                objectString = objectPathString.substr(0, rootObjIndex);
    Value object = scope->get(StringData(objectString));
    Document doc = object.getDocument();
    BSONObj obj = doc.toBson();
    BSONElement el = obj.getFieldDotted(fieldPathString);
    return Value(el);
}


std::string ObjectAccessorOperator::generateNestedField(const Node* head, Scope* scope) const {
    std::string cur = (head->getName()).rawData();
    std::string leftNestedField, rightNestedField;
    if (cur == ".") {
        leftNestedField = ObjectAccessorOperator::generateNestedField(
            (checked_cast<const BinaryOperator*>(head))->getLeftChild(), scope);
        rightNestedField = ObjectAccessorOperator::generateNestedField(
            (checked_cast<const BinaryOperator*>(head))->getRightChild(), scope);
    } else if (cur == "[") {
        cur = ".";
        leftNestedField = ObjectAccessorOperator::generateNestedField(
            (checked_cast<const BinaryOperator*>(head))->getLeftChild(), scope);
        Value rightChildValue =
            (checked_cast<const BinaryOperator*>(head))->getRightChild()->evaluate(scope);
        rightNestedField = rightChildValue.coerceToString();
    } else {
        leftNestedField = "";
        rightNestedField = "";
    }
    return leftNestedField + cur + rightNestedField;
}

/*StringData ObjectAccessorOperator::getName() const override {
    return _objectPathString;
}*/

}  // namespace tinyjs
}  // namespace mongo
