#ifndef OPERATOR_NODE_H
#define OPERATOR_NODE_H

#include "mongo/db/pipeline/value.h"
#include "mongo/scripting/tinyjs/node.h"
#include <unordered_map>

namespace mongo {
namespace tinyjs {

class ClauseNode: public Node {
public:
    ClauseNode(): Node("ClauseNode") {};
};

class LeafNode: public Node {
public:
    LeafNode(Token token): Node("LeafNode") {
        this->type = token.type;
        this->value = token.value;
    };
private:
    TokenType type;
    StringData value;
};

class VariableNode: public Node {
public:
    VariableNode(): Node("VariableNode") {};
};

class ObjectNode: public Node {
public:
    ObjectNode(): Node("ObjectNode") {};
};

class ObjectAccessorNode: public Node {
public:
    ObjectAccessorNode(): Node("ObjectAccessorNode") {};
};

class TermNode: public Node {
public:
    TermNode(): Node("TermNode") {};
};

class ArrayElementNode: public Node {
public:
    ArrayElementNode(): Node("ArrayElementNode") {};
};

class ArrayLiteralNode: public Node {
public:
    ArrayLiteralNode(): Node("ArrayLiteralNode") {};
};

class ArrayTailNode: public Node {
public:
    ArrayTailNode(): Node("ArrayTailNode") {};
};

class ArrayIndexedNode: public Node {
public:
    ArrayIndexedNode(): Node("ArrayIndexedNode") {};
};

class FactorNode: public Node {
public:
    FactorNode(): Node("FactorNode") {};
};

class MultiplicativeExpressionNode: public Node {
public:
    MultiplicativeExpressionNode(): Node("MultiplicativeExpressionNode") {};
};

class MultiplicativeOperationNode: public Node {
public:
    MultiplicativeOperationNode(): Node("MultiplicativeOperationNode") {};
};

class ArithmeticExpressionNode: public Node {
public:
    ArithmeticExpressionNode(): Node("ArithmeticExpressionNode") {};
};

class ArithmeticOperationNode: public Node {
public:
    ArithmeticOperationNode(): Node("ArithmeticOperationNode") {};
};

class BooleanFactorNode: public Node {
public:
    BooleanFactorNode(): Node("BooleanFactorNode") {};
};

class RelationalExpressionNode: public Node {
public:
    RelationalExpressionNode(): Node("RelationalExpressionNode") {};
};

class RelationalOperationNode: public Node {
public:
    RelationalOperationNode(): Node("RelationalOperationNode") {};
};

class BooleanExpressionNode: public Node {
public:
    BooleanExpressionNode(): Node("BooleanExpressionNode") {};
};

class BooleanOperationNode: public Node {
public:
    BooleanOperationNode(): Node("BooleanOperationNode") {};
};

class TernaryOperationNode: public Node {
public:
    TernaryOperationNode(): Node("TernaryOperationNoder") {};
};

class ReturnStatementNode: public Node {
public:
    ReturnStatementNode(): Node("ReturnStatementNode") {};
};

class LogicalOperationNode: public Node {
public:
    LogicalOperationNode(): Node("LogicalOperationNode") {};
};

class ComparisonOperationNode: public Node {
public:
    ComparisonOperationNode(): Node("ComparisonOperationNode") {};
};

}
}
#endif
