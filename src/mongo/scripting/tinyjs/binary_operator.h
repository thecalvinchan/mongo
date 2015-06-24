#ifndef BINARY_OPERATOR_H
#define BINARY_OPERATOR_H

#include "operator.h"

namespace mongo {

class BinaryOperator: public OperatorNode {
public:
    BinaryOperator();
    ~BinaryOperator();
    Node *getLChild() const;
    Node *setLChild(Node *node);
    Node *getRChild() const;
    Node *setRChild(Node *node);
private:
    Node *_lChild;
    Node *_rChild;
};

BinaryOperator::~BinaryOperator() {
    delete this->_lChild;
    delete this->_rChild;
}

}

#endif
