#include "mongo/scripting/tinyjs/binary_operator.h"

namespace mongo {
namespace tinyjs {

ArrayLiteral::ArrayLiteral(TokenType type) : NonTerminalNode(type) {
    children = NULL;
}

std::vector<Node *>* const ArrayLiteral::getChildren() {
    return &_children;
}

void ArrayLiteral::setChildren(std::vector<std::unique_ptr<Node> > children) {
    _children = children;
}

}
}
