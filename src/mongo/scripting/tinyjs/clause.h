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

#include <unordered_map>

#include "mongo/scripting/tinyjs/node.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace tinyjs {

class NonTerminalNode : public Node {
public:
    std::string getName();
};
std::string NonTerminalNode::getName() {
    return _name;
}

class TerminalNode : public Node {
public:
    TerminalNode(Token token) : Node("TerminalNode") {
        _type = token.type;
        _value = token.value;
    };

    std::string getName() {
        return _names[static_cast<int>(_type)];
    };

private:
    TokenType _type;
    StringData _value;
    static const std::string _names[];
};

const std::string TerminalNode::_names[] = {
    "ThisIdentifier",
    "ReturnKeyword",
    "NullLiteral",
    "UndefinedLiteral",
    "FunctionKeyword",
    "IntegerLiteral",
    "FloatLiteral",
    "BooleanLiteral",
    "StringLiteral",
    "Identifier",
    "Add",
    "Subtract",
    "Multiply",
    "Divide",
    "TripleEquals",
    "DoubleEquals",
    "LessThan",
    "LessThanEquals",
    "GreaterThan",
    "GreaterThanEquals",
    "NotEquals",
    "DoubleNotEquals",
    "LogicalAnd",
    "LogicalOr",
    "LogicalNot",
    "SemiColon",
    "OpenParen",
    "CloseParen",
    "QuestionMark",
    "Colon",
    "Period",
    "Comma",
    "OpenSquareBracket",
    "CloseSquareBracket",
    "OpenCurlyBrace",
    "CloseCurlyBrace"
};

class ClauseNode : public NonTerminalNode {
public:
    ClauseNode() : NonTerminalNode("ClauseNode"){};
};

class VariableNode : public NonTerminalNode {
public:
    VariableNode() : NonTerminalNode("VariableNode"){};
};

class ObjectNode : public NonTerminalNode {
public:
    ObjectNode() : NonTerminalNode("ObjectNode"){};
};

class ObjectAccessorNode : public NonTerminalNode {
public:
    ObjectAccessorNode() : NonTerminalNode("ObjectAccessorNode"){};
};

class TermNode : public NonTerminalNode {
public:
    TermNode() : NonTerminalNode("TermNode"){};
};

class ArrayElementNode : public NonTerminalNode {
public:
    ArrayElementNode() : NonTerminalNode("ArrayElementNode"){};
};

class ArrayLiteralNode : public NonTerminalNode {
public:
    ArrayLiteralNode() : NonTerminalNode("ArrayLiteralNode"){};
};

class ArrayTailNode : public NonTerminalNode {
public:
    ArrayTailNode() : NonTerminalNode("ArrayTailNode"){};
};

class ArrayIndexedNode : public NonTerminalNode {
public:
    ArrayIndexedNode() : NonTerminalNode("ArrayIndexedNode"){};
};

class FactorNode : public NonTerminalNode {
public:
    FactorNode() : NonTerminalNode("FactorNode"){};
};

class MultiplicativeExpressionNode : public NonTerminalNode {
public:
    MultiplicativeExpressionNode() : NonTerminalNode("MultiplicativeExpressionNode"){};
};

class MultiplicativeOperationNode : public NonTerminalNode {
public:
    MultiplicativeOperationNode() : NonTerminalNode("MultiplicativeOperationNode"){};
};

class ArithmeticExpressionNode : public NonTerminalNode {
public:
    ArithmeticExpressionNode() : NonTerminalNode("ArithmeticExpressionNode"){};
};

class ArithmeticOperationNode : public NonTerminalNode {
public:
    ArithmeticOperationNode() : NonTerminalNode("ArithmeticOperationNode"){};
};

class BooleanFactorNode : public NonTerminalNode {
public:
    BooleanFactorNode() : NonTerminalNode("BooleanFactorNode"){};
};

class RelationalExpressionNode : public NonTerminalNode {
public:
    RelationalExpressionNode() : NonTerminalNode("RelationalExpressionNode"){};
};

class RelationalOperationNode : public NonTerminalNode {
public:
    RelationalOperationNode() : NonTerminalNode("RelationalOperationNode"){};
};

class BooleanExpressionNode : public NonTerminalNode {
public:
    BooleanExpressionNode() : NonTerminalNode("BooleanExpressionNode"){};
};

class BooleanOperationNode : public NonTerminalNode {
public:
    BooleanOperationNode() : NonTerminalNode("BooleanOperationNode"){};
};

class TernaryOperationNode : public NonTerminalNode {
public:
    TernaryOperationNode() : NonTerminalNode("TernaryOperationNoder"){};
};

class ReturnStatementNode : public NonTerminalNode {
public:
    ReturnStatementNode() : NonTerminalNode("ReturnStatementNode"){};
};

class LogicalOperationNode : public NonTerminalNode {
public:
    LogicalOperationNode() : NonTerminalNode("LogicalOperationNode"){};
};

class ComparisonOperationNode : public NonTerminalNode {
public:
    ComparisonOperationNode() : NonTerminalNode("ComparisonOperationNode"){};
};
}
}
