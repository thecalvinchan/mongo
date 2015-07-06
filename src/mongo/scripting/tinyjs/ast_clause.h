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

#pragma once

#include "mongo/scripting/tinyjs/ast_node.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace tinyjs {

class NonTerminalNode : public Node {
public:
    NonTerminalNode(std::string name) : Node(name) {}
    virtual ~NonTerminalNode() = 0;
    std::string getName() {
        return _name;
    }
    //getValue and getChildren are only used for testing
    std::string getValue() {;
        std::string res = getName();
        std::vector<std::unique_ptr<Node> >* children = this->getChildren();
        for (std::vector<std::unique_ptr<Node> >::iterator it = children->begin();
             it != children->end(); it++) {
            res += " ";
            res += ((*it).get())->getValue();
        }
        return res;
    }
    virtual std::vector<std::unique_ptr<Node> >* getChildren() = 0;
};

class UnaryOperator : public NonTerminalNode {
public:
    UnaryOperator();
    ~UnaryOperator();
    std::vector<std::unique_ptr<Node> >* getChildren();
    std::unique_ptr<Node> getChild();
private:
    std::unique_ptr<Node> child;
};

class BinaryOperator : public NonTerminalNode {
public:
    BinaryOperator();
    ~BinaryOperator();
    std::vector<std::unique_ptr<Node> >* getChildren();
    std::unique_ptr<Node> getLeftChild();
    std::unique_ptr<Node> getRightChild();
private:
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> rightChild;
};

class TernaryOperator : public NonTerminalNode {
public:
    TernaryOperator();
    ~TernaryOperator();
    std::vector<std::unique_ptr<Node> >* getChildren();
    std::unique_ptr<Node> getLeftChild();
    std::unique_ptr<Node> getMiddleChild();
    std::unique_ptr<Node> getRightChild();
private:
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> middleChild;
    std::unique_ptr<Node> rightChild;
};

class TerminalNode : public Node {
};

class NullLiteralNode : public TerminalNode {
};

class UndefinedLiteralNode : public TerminalNode {
};

class IntegerLiteralNode : public TerminalNode {
};

class FloatLiteralNode : public TerminalNode {
};

class BooleanLiteralNode : public TerminalNode {
};

class StringLiteralNode : public TerminalNode {
};

class IdentifierNode : public TerminalNode {
};

}
}
