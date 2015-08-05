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
#include "mongo/scripting/tinyjs/greater_than_operator.h"


namespace mongo {
namespace tinyjs {

GreaterThanOperator::GreaterThanOperator() : BinaryOperator(TokenType::kGreaterThan) {}

const Value GreaterThanOperator::evaluate(Scope* scope, Value& returnValue) const {
    if (!returnValue.nullish()) {
        return returnValue;
    }
    Value leftValue = this->getLeftChild()->evaluate(scope, returnValue);
    Value rightValue = this->getRightChild()->evaluate(scope, returnValue);
    return Value(Value::compare(leftValue, rightValue) > 0);
}

bool GreaterThanOperator::optimizable(bool optimize) const {
    bool leftChildOptimizable = this->getLeftChild()->optimizable();
    bool rightChildOptimizable = this->getRightChild()->optimizable();
    if (!(leftChildOptimizable && rightChildOptimizable)) {
        //only zero or one branch contains object accessor
        if (optimize) {
            //optimize query here
        }
    }
    return leftChildOptimizable || rightChildOptimizable;
}

}  // namespace tinyjs
}  // namespace mongo
