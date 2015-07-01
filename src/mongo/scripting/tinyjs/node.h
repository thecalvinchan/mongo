#include <vector>
#include <string>
#include <memory>
#include "mongo/db/pipeline/value.h"
#include "mongo/scripting/tinyjs/lexer.h"

#ifndef NODE_H
#define NODE_H

namespace mongo {
namespace tinyjs {
class Node {
public:
    Node(std::string name);
    ~Node();
    Value *returnValue() const;
    void addChild(std::unique_ptr<Node> child);
    std::vector<std::unique_ptr<Node> > const& getChildren();
    std::string getName();
private:    
    std::string name;
    std::vector<std::unique_ptr<Node> > children;
};

}
}

#endif
