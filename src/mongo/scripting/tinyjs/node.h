#include <vector>
#include <string>
#include "mongo/db/pipeline/value.h"
#include "mongo/scripting/tinyjs/lexer.h"

#ifndef NODE_H
#define NODE_H

namespace mongo {
namespace tinyjs {
class Node {
public:
    Node(Token token);
    ~Node();
    Value *returnValue() const;
    void addChild(std::unique_ptr<Node> child);
    std::vector<std::unique_ptr<Node> getChildren();
private:    
    TokenType type;
    std::string value;
    std::vector<std::unique_ptr<Node> > children;
};

}
}

#endif
