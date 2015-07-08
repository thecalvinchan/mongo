#include "mongo/scripting/tinyjs/array_literal.h"
#include "mongo/base/checked_cast.h"

namespace mongo {
namespace tinyjs {

ArrayLiteral::ArrayLiteral(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node *>* ArrayLiteral::getChildren() const {
    return NULL;
}

void ArrayLiteral::setChildren(std::vector<std::unique_ptr<Node> > children) {
    _children = children;
}

}
}
