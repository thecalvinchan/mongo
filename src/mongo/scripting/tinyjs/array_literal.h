#pragma once

#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

class ArrayLiteral : public NonTerminalNode {
public:
    ArrayLiteral(TokenType type);
    ~BinaryOperator();
    std::vector<std::unique_ptr<Node> >* getChildren();
    void setChildren(std::vector<std::unique_ptr<Node> >*);
private:
    std::vector<std::unique_ptr<Node> > _children;
};

}
}
