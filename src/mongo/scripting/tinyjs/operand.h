#ifndef OPERAND_NODE_H
#define OPERAND_NODE_H

#include "node.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {

class OperandNode : public Node {
public:
    enum OperandType {
        INTEGER_L,
        FLOAT_L,
        BOOLEAN_L,
        STRING_L,
        OBJECT,
    };
    OperandNode(Value *val, OperandType type);
    ~OperandNode();
    Value *returnValue() const;
private:
    OperandType _operandType;
    Value *_value;
};

}

#endif
