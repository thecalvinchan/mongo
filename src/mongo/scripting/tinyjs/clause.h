#ifndef OPERATOR_NODE_H
#define OPERATOR_NODE_H

#include "mongo/db/pipeline/value.h"
#include "mongo/scripting/tinyjs/node.h"
#include <unordered_map>

namespace mongo {
namespace tinyjs {

class ClauseNode: public Node {
};

class VariableNode: public Node {
}

class ObjectNode: public Node {
}

class ObjectAccessorNode: public Node {
}

class TermNode: public Node {
}

class ArrayElementNode: public Node {
}

class ArrayLiteralNode: public Node {
}

class ArrayTailNode: public Node {
}

class ArrayIndexedNode: public Node {
}

class FactorNode: public Node {
}

class MultiplicativeExpressionNode: public Node {
}

class MultiplicativeOperationNode: public Node {
}

class ArithmeticExpressionNode: public Node {
}

class ArithmeticOperationNode: public Node {
}

class BooleanFactorNode: public Node {
}

class RelationalExpressionNode: public Node {
}

class RelationalOperationNode: public Node {
}

class BooleanExpressionNode: public Node {
}

class BooleanOperationNode: public Node {
}

class TernaryOperationNode: public Node {
}

class ReturnStatementNode: public Node {
}

class LogicalOperationNode: public Node {
}

class ComparisonOperationNode: public Node {
}

}
}
#endif
