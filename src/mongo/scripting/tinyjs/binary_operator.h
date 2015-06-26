#ifndef BINARY_OPERATOR_H
#define BINARY_OPERATOR_H

#include "operator.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {

class BinaryOperator: public OperatorNode {
public:
    enum BinaryOperatorType {
        OBJ_ACCESSOR,
        MULTIPLICATIVE,
        ADDITIVE,
        COMPARISON,
        LOGICAL,
    };
    BinaryOperator(Value *val, BinaryOperatorType type);
    ~BinaryOperator();
    Node *getLChild() const;
    Node *setLChild(Node *node);
    Node *getRChild() const;
    Node *setRChild(Node *node);
    BinaryOperatorType getBinaryOpType();
private:
    Node *_lChild;
    Node *_rChild;
    BinaryOperatorType _binaryOpType;
};

BinaryOperator::~BinaryOperator() {
    delete this->_lChild;
    delete this->_rChild;
}

}

#endif
