#pragma once

#include "mongo/scripting/tinyjs/node.h"

namespace mongo {
namespace tinyjs {

class NonTerminalNode : public Node {
public:
    NonTerminalNode(TokenType type);
    virtual ~NonTerminalNode() = 0;
    //getValue and getChildren are only used for testing
    virtual std::vector<Node*> getChildren() = 0;
    Value* getValue() const;
    std::string getName() const;
    TokenType getType();
private:
    TokenType _type;
    static const std::map<TokenType, std::string> _values;
};

}
}
