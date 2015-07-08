#pragma once

#include "mongo/scripting/tinyjs/node.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace tinyjs {

class NonTerminalNode : public Node {
public:
    NonTerminalNode(TokenType type);
    virtual ~NonTerminalNode() {};
    //getValue and getChildren are only used for testing
    virtual std::vector<Node*> getChildren() = 0;
    const Value* getValue() const;
    std::string getName() const;
    TokenType getType();
    const static std::string names[];
private:
    TokenType _type;
};

}
}
