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

GreaterThanOperator::optimizable(bool optimize, std::unique_ptr<AndMatchExpression> root) const {
    bool leftChildOptimizable = this->getLeftChild()->optimizable(optimize, root)
    bool rightChildOptimizable = this->getRightChild()->optimizable(optimize, root)
    if (!(leftChildOptimizable && rightChildOptimizable)) {
        //only zero or one branch contains object accessor
        if (optimize) {
            // optimize query here
            if (!(leftChildOptimizable && rightChildOptimizable)) {
                // case where both sides are constant ]
                // TODO
            } else if (leftChildOptimizable) {
                // case where left child contains an object access and right child doesn't
                // TODO: for now, just assume that left child is one simple object access
                BSONElement constant = BSONElement(rightChild->evaluate().getInt());  // TODO
                std::string fieldName = (checked_cast<ObjectAccessor*>(leftChild.get()))->generateNestedField();
                std::unique_ptr<ComparisonMatchExpression> eq =
                    stdx::make_unique<ComparisonMatchExpression>(GT);
                Status s = eq->init(fieldName, constant);
                if (!s.isOK())
                    return s;

                root->add(eq.release());
            } else {
                // case where right child contains an object access and left child doesn't
                // TODO: for now, just assume that left child is one simple object access
                BSONElement constant = BSONElement(leftChild->evaluate().getInt());  // TODO
                std::string fieldName = (checked_cast<ObjectAccessor*>(rightChild.get()))->generateNestedField();
                std::unique_ptr<ComparisonMatchExpression> eq =
                    stdx::make_unique<ComparisonMatchExpression>(GT);
                Status s = eq->init(fieldName, constant);
                if (!s.isOK())
                    return s;

                root->add(eq.release());
            }
        }
    }
    return leftChildOptimizable || rightChildOptimizable;
}

}  // namespace tinyjs
}  // namespace mongo
