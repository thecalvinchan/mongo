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
ClauseNode::ClauseNode(Token token) : Node(token) {
}

class LeafNode: public Node {
public:
    LeafNode(Token token);
};
LeafNode::LeafNode(Token token) : Node(token) {
}

class VariableNode: public Node {
public:
    VariableNode(Token token);
};
VariableNode::VariableNode(Token token) : Node(token) {
}

class ObjectNode: public Node {
public:
    ObjectNode(Token token);
};
ObjectNode::ObjectNode(Token token) : Node(token) {
}

class ObjectAccessorNode: public Node {
public:
    ObjectAccessorNode(Token token);
};
ObjectAccessorNode::ObjectAccessorNode(Token token) : Node(token) {
}

class TermNode: public Node {
public:
    TermNode(Token token);
};
TermNode::TermNode(Token token) : Node(token) {
}

class ArrayElementNode: public Node {
public:
    ArrayElementNode(Token token);
};
ArrayElementNode::ArrayElementNode(Token token) : Node(token) {
}

class ArrayLiteralNode: public Node {
public:
    ArrayLiteralNode(Token token);
};
ArrayLiteralNode::ArrayLiteralNode(Token token) : Node(token) {
}

class ArrayTailNode: public Node {
public:
    ArrayTailNode(Token token);
};
ArrayTailNode::ArrayTailNode(Token token) : Node(token) {
}

class ArrayIndexedNode: public Node {
public:
    ArrayIndexedNode(Token token);
};
ArrayIndexedNode::ArrayIndexedNode(Token token) : Node(token) {
}

class FactorNode: public Node {
public:
    FactorNode(Token token);
};
FactorNode::FactorNode(Token token) : Node(token) {
}

class MultiplicativeExpressionNode: public Node {
public:
    MultiplicativeExpressionNode(Token token);
};
MultiplicativeExpressionNode::MultiplicativeExpressionNode(Token token) : Node(token) {
}

class MultiplicativeOperationNode: public Node {
public:
    MultiplicativeOperationNode(Token token);
};
MultiplicativeOperationNode::MultiplicativeOperationNode(Token token) : Node(token) {
}

class ArithmeticExpressionNode: public Node {
public:
    ArithmeticExpressionNode(Token token);
};
ArithmeticExpressionNode::ArithmeticExpressionNode(Token token) : Node(token) {
}

class ArithmeticOperationNode: public Node {
public:
    ArithmeticOperationNode(Token token);
};
ArithmeticOperationNode::ArithmeticOperationNode(Token token) : Node(token) {
}

class BooleanFactorNode: public Node {
public:
    BooleanFactorNode(Token token);
};
BooleanFactorNode::BooleanFactorNode(Token token) : Node(token) {
}

class RelationalExpressionNode: public Node {
public:
    RelationalExpressionNode(Token token);
};
RelationalExpressionNode::RelationalExpressionNode(Token token) : Node(token) {
}

class RelationalOperationNode: public Node {
public:
    RelationalOperationNode(Token token);
};
RelationalOperationNode::RelationalOperationNode(Token token) : Node(token) {
}

class BooleanExpressionNode: public Node {
public:
    BooleanExpressionNode(Token token);
};
BooleanExpressionNode::BooleanExpressionNode(Token token) : Node(token) {
}

class BooleanOperationNode: public Node {
public:
    BooleanOperationNode(Token token);
};
BooleanOperationNode::BooleanOperationNode(Token token) : Node(token) {
}

class TernaryOperationNode: public Node {
public:
    TernaryOperationNode(Token token);
};
TernaryOperationNode::TernaryOperationNode(Token token) : Node(token) {
}

class ReturnStatementNode: public Node {
public:
    ReturnStatementNode(Token token);
};
ReturnStatementNode::ReturnStatementNode(Token token) : Node(token) {
}

class LogicalOperationNode: public Node {
public:
    LogicalOperationNode(Token token);
};
LogicalOperationNode::LogicalOperationNode(Token token) : Node(token) {
}

class ComparisonOperationNode: public Node {
public:
    ComparisonOperationNode(Token token);
};
ComparisonOperationNode::ComparisonOperationNode(Token token) : Node(token) {
}

}
}
#endif
