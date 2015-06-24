#ifndef OPERATOR_NODE_H
#define OPERATOR_NODE_H

#include "mongo/db/pipeline/value.h"
#include "node.h"

namespace mongo {

class OperatorNode: public Node {
public:
    OperatorNode();
    virtual ~OperatorNode();
};

}

#endif
