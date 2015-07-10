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

#include "mongo/scripting/tinyjs/ternary_operator.h"

namespace mongo {
namespace tinyjs {

TernaryOperator::TernaryOperator(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node*> TernaryOperator::getChildren() const {
    std::vector<Node* > children;
    children.push_back(_leftChild.get());
    children.push_back(_middleChild.get());
    children.push_back(_rightChild.get());
    return children;
}

Node* TernaryOperator::getLeftChild() const {
    return _leftChild.get();
}

Node* TernaryOperator::getMiddleChild() const {
    return _middleChild.get();
}

Node* TernaryOperator::getRightChild() const {
    return _rightChild.get();
}

void TernaryOperator::setLeftChild(std::unique_ptr<Node> node) {
    _leftChild = std::move(node);
}

void TernaryOperator::setMiddleChild(std::unique_ptr<Node> node) {
    _middleChild = std::move(node);
}

void TernaryOperator::setRightChild(std::unique_ptr<Node> node) {
    _rightChild = std::move(node);
}

const Value* TernaryOperator::evaluate(Scope* scope) const {
    
    const Value* condition = this->getLeftChild()->evaluate(scope);

    if (condition->getType() != Bool) { // TODO how does it understand the BSON types?
        //TODO throw error
    }

    if (condition->getBool()) {
        // Evaluate the first branch
        return this->getMiddleChild()->evaluate(scope);
    } else {
        // Evaluate the second branch
        return this->getRightChild()->evaluate(scope);
    }
}

} // namespace tinyjs
} // namespace mongo
