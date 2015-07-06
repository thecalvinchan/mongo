#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

NonTerminalNode::NonTerminalNode(TokenType type) : type(type) {
}

std::string NonTerminalNode::getValue() {
    std::string res = getName();
    std::vector<std::unique_ptr<Node> >* children = this->getChildren();
    for (std::vector<std::unique_ptr<Node> >::iterator it = children->begin();
         it != children->end(); it++) {
        res += " ";
        res += ((*it).get())->getValue();
    }
    return res;
}

TokenType NonTerminalNode::getType() {
    return type;
}

}
}
