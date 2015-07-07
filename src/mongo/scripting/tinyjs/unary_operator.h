#pragma once

#include "mongo/scripting/tinyjs/nonterminal_node.h"
#include <vector>

namespace mongo {
namespace tinyjs {

class UnaryOperator : public NonTerminalNode {
public:
    UnaryOperator(TokenType type);
    ~UnaryOperator();
    std::vector<Node*>* getChildren() const;
    Node* getChild();
    void setChild(std::unique_ptr<Node> node);
private:
    std::unique_ptr<Node> child;
};

}
}
