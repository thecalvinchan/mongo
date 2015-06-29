#ifndef OPERAND_NODE_H
#define OPERAND_NODE_H

#include "node.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {

class Terminal: public Node {
public:
    enum TerminalType {
        INTEGER_L,
        FLOAT_L,
        BOOLEAN_L,
        STRING_L,
        OBJECT,
        IDENTIFIER,
        FUNCTION_DEC = Node::kFunctionDec,
    };
    Terminal(Value *val, OperandType type);
    ~Terminal();
    Value *returnValue() const;
private:
    TerminalType _operandType;
    Value *_value;
};

}

#endif
