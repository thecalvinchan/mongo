#include "operator.h"

namespace mongo {

class UnaryOperator: public OperatorNode {
public:
    enum UnaryOperatorType {
        RETURN,
        IDENTIFIER,
    };
    UnaryOperator(Value *val, UnaryOperatorType type);
    virtual ~UnaryOperator();
    Node *getChild() const;
    Node *setChild(Node *node);
    Value *returnValue() const;
private:
    Node *_child;
};

UnaryOperator::~UnaryOperator() {
    delete this->_child;
}

}
