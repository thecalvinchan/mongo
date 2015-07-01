#include "mongo/db/pipeline/value.h"
#include <vector>

#ifndef NODE_H
#define NODE_H

namespace mongo {
namespace tinyjs {
class Node {
public:
    Node();
    ~Node();
    virtual Value *returnValue() const;
    void addChild(std::unique_ptr<Node> child);
private:    
    virtual static TokenType type;
    std::vector<std::unique_ptr<Node> > children;
};

}
}

#endif
