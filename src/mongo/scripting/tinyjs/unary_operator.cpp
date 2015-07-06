#include "mongo/scripting/tinyjs/unary_operator.h"

namespace mongo {
namespace tinyjs {

UnaryOperator::UnaryOperator(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node*>* getChildren() {
    std::vector<std::unique_ptr<Node> > children;
    children.push_back(child.get());
    return &children;
}

Node* getChild() {
    return child.get();
}

void UnaryOperator::setChild(std::unique_ptr<Node> node) {
    child = std::move(node);
}

}
}
