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

class ClauseNode : public Node {
public:
    ClauseNode() : Node("ClauseNode"){};
};

class LeafNode : public Node {
public:
    LeafNode(Token token) : Node("LeafNode") {
        _type = token.type;
        _value = token.value;
    };

private:
    TokenType _type;
    StringData _value;
};

class VariableNode : public Node {
public:
    VariableNode() : Node("VariableNode"){};
};

class ObjectNode : public Node {
public:
    ObjectNode() : Node("ObjectNode"){};
};

class ObjectAccessorNode : public Node {
public:
    ObjectAccessorNode() : Node("ObjectAccessorNode"){};
};

class TermNode : public Node {
public:
    TermNode() : Node("TermNode"){};
};

class ArrayElementNode : public Node {
public:
    ArrayElementNode() : Node("ArrayElementNode"){};
};

class ArrayLiteralNode : public Node {
public:
    ArrayLiteralNode() : Node("ArrayLiteralNode"){};
};

class ArrayTailNode : public Node {
public:
    ArrayTailNode() : Node("ArrayTailNode"){};
};

class ArrayIndexedNode : public Node {
public:
    ArrayIndexedNode() : Node("ArrayIndexedNode"){};
};

class FactorNode : public Node {
public:
    FactorNode() : Node("FactorNode"){};
};

class MultiplicativeExpressionNode : public Node {
public:
    MultiplicativeExpressionNode() : Node("MultiplicativeExpressionNode"){};
};

class MultiplicativeOperationNode : public Node {
public:
    MultiplicativeOperationNode() : Node("MultiplicativeOperationNode"){};
};

class ArithmeticExpressionNode : public Node {
public:
    ArithmeticExpressionNode() : Node("ArithmeticExpressionNode"){};
};

class ArithmeticOperationNode : public Node {
public:
    ArithmeticOperationNode() : Node("ArithmeticOperationNode"){};
};

class BooleanFactorNode : public Node {
public:
    BooleanFactorNode() : Node("BooleanFactorNode"){};
};

class RelationalExpressionNode : public Node {
public:
    RelationalExpressionNode() : Node("RelationalExpressionNode"){};
};

class RelationalOperationNode : public Node {
public:
    RelationalOperationNode() : Node("RelationalOperationNode"){};
};

class BooleanExpressionNode : public Node {
public:
    BooleanExpressionNode() : Node("BooleanExpressionNode"){};
};

class BooleanOperationNode : public Node {
public:
    BooleanOperationNode() : Node("BooleanOperationNode"){};
};

class TernaryOperationNode : public Node {
public:
    TernaryOperationNode() : Node("TernaryOperationNoder"){};
};

class ReturnStatementNode : public Node {
public:
    ReturnStatementNode() : Node("ReturnStatementNode"){};
};

class LogicalOperationNode : public Node {
public:
    LogicalOperationNode() : Node("LogicalOperationNode"){};
};

class ComparisonOperationNode : public Node {
public:
    ComparisonOperationNode() : Node("ComparisonOperationNode"){};
};
}
}
