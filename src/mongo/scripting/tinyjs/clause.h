#ifndef OPERATOR_NODE_H
#define OPERATOR_NODE_H

#include "mongo/db/pipeline/value.h"
#include "mongo/scripting/tinyjs/node.h"
#include <unordered_map>

namespace mongo {
namespace tinyjs {

class ClauseNode: public Node {
public:
    ClauseNode(Token token);
};

class LeafNode: public Node {
public:
    LeafNode(Token token);
};

class VariableNode: public Node {
public:
    VariableNode(Token token);
};

class ObjectNode: public Node {
public:
    ObjectNode(Token token);
};

class ObjectAccessorNode: public Node {
public:
    ObjectAccessorNode(Token token);
};

class TermNode: public Node {
public:
    TermNode(Token token);
};

class ArrayElementNode: public Node {
public:
    ArrayElementNode(Token token);
};

class ArrayLiteralNode: public Node {
public:
    ArrayLiteralNode(Token token);
};

class ArrayTailNode: public Node {
public:
    ArrayTailNode(Token token);
};

class ArrayIndexedNode: public Node {
public:
    ArrayIndexedNode(Token token);
};

class FactorNode: public Node {
public:
    FactorNode(Token token);
};

class MultiplicativeExpressionNode: public Node {
public:
    MultiplicativeExpressionNode(Token token);
};

class MultiplicativeOperationNode: public Node {
public:
    MultiplicativeOperationNode(Token token);
};

class ArithmeticExpressionNode: public Node {
public:
    ArithmeticExpressionNode(Token token);
};

class ArithmeticOperationNode: public Node {
public:
    ArithmeticOperationNode(Token token);
};

class BooleanFactorNode: public Node {
public:
    BooleanFactorNode(Token token);
};

class RelationalExpressionNode: public Node {
public:
    RelationalExpressionNode(Token token);
};

class RelationalOperationNode: public Node {
public:
    RelationalOperationNode(Token token);
};

class BooleanExpressionNode: public Node {
public:
    BooleanExpressionNode(Token token);
};

class BooleanOperationNode: public Node {
public:
    BooleanOperationNode(Token token);
};

class TernaryOperationNode: public Node {
public:
    TernaryOperationNode(Token token);
};

class ReturnStatementNode: public Node {
public:
    ReturnStatementNode(Token token);
};

class LogicalOperationNode: public Node {
public:
    LogicalOperationNode(Token token);
};

class ComparisonOperationNode: public Node {
public:
    ComparisonOperationNode(Token token);
};

}
}
#endif
