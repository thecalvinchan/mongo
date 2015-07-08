#include "mongo/scripting/tinyjs/array_literal.h"
#include "mongo/base/checked_cast.h"

namespace mongo {
namespace tinyjs {

ArrayLiteral::ArrayLiteral(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node *> ArrayLiteral::getChildren() {
    std::vector<Node*> children;
    for (std::size_t i = 0; i < _children.size(); i++) {
        children.push_back(_children[i].get());
    }
    return children;
}

void ArrayLiteral::setChild(std::unique_ptr<Node> child) {
    _children.push_back(std::move(child));
}

}
}
