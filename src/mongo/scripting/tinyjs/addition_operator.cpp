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

#include "mongo/scripting/tinyjs/binary_operator.h"
#include "mongo/scripting/tinyjs/addition_operator.h"


namespace mongo {
namespace tinyjs {

AdditionOperator::AdditionOperator() : BinaryOperator(TokenType::kAdd) {}

const Value AdditionOperator::evaluate(Scope* scope, Value& returnValue) const {
    if (!returnValue.nullish()) {
        return returnValue;
    }
    Value leftValue = this->getLeftChild()->evaluate(scope, returnValue);
    Value rightValue = this->getRightChild()->evaluate(scope, returnValue);

    if (isString(leftValue)) {
        return Value(makeString(leftValue) + makeString(rightValue));
    } else if (isString(rightValue)) {
        return Value(makeString(leftValue) + makeString(rightValue));
    } else if ((leftValue.getType() == Array) || (rightValue.getType() == Array)) {
        return Value(makeString(leftValue) + makeString(rightValue));
    }

    if (!countsAsNumber(leftValue) || !countsAsNumber(rightValue)) {
        return Value(std::nan(""));
    }

    leftValue = makeNumeric(leftValue);
    rightValue = makeNumeric(rightValue);

    if (leftValue.getType() == NumberDouble) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getDouble() + rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getDouble() + rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getDouble() + rightValue.getLong());
        } else {
            return Value(std::nan(""));
        }
    } else if (leftValue.getType() == NumberInt) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getInt() + rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getInt() + rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getInt() + rightValue.getLong());
        } else {
            return Value(std::nan(""));
        }
    } else if (leftValue.getType() == NumberLong) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getLong() + rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getLong() + rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getLong() + rightValue.getLong());
        } else {
            return Value(std::nan(""));
        }
    } else {
        return Value(std::nan(""));
    }
}

}  // namespace tinyjs
}  // namespace mongo
