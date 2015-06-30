#include "mongo/db/pipeline/value.h"

#ifndef NODE_H
#define NODE_H

namespace mongo {

class Node {
public:
    Node();
    virtual ~Node();
    virtual Value *returnValue() const;
private:    
    virtual static NodeType type;
    virtual static bool terminal;
};

}

#endif
