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

#include "mongo/scripting/tinyjs/unary_operator.h"

namespace mongo {
namespace tinyjs {

UnaryOperator::UnaryOperator(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node*> UnaryOperator::getChildren() const {
    std::vector<Node*> children;
    children.push_back(_child.get());
    return children;
}

Node* UnaryOperator::getChild() const {
    return _child.get();
}

void UnaryOperator::setChild(std::unique_ptr<Node> node) {
    _child = std::move(node);
}

const Value UnaryOperator::evaluate(Scope* scope) const {
    switch (this->getType()) {
        case TokenType::kReturnKeyword: 
            return evaluateReturn(scope);
            break;
        case TokenType::kSubtract: 
            return evaluateNegativeOperator(scope);
            break;
        default:
            return Value(); //TODO: error?
    }
}

const Value UnaryOperator::evaluateReturn(Scope* scope) const {
    return (_child->evaluate(scope));
}

const Value UnaryOperator::evaluateNegativeOperator(Scope* scope) const {
    const Value childValue = getChild()->evaluate(scope);
    switch (childValue.getType()) {
        case NumberDouble: {
            double val = -1 * childValue.getDouble();
            return Value(val);
            break;
        }
        case NumberInt: {
            int val = -1 * childValue.getInt();
            return Value(val);
            break;
        }
        case NumberLong: {
            long long val = -1 * childValue.getLong();
            return Value(val);
            break;
        }
        case Bool: {
            return childValue.getBool() ? Value(-1) : Value(-0);
        }
        default: {
            // Actual behavior
            throw std::runtime_error("NaN");
        }
    }
}

} // namespace tinyjs
} // namespace mongo
