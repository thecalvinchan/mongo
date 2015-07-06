#include "mongo/scripting/tinyjs/binary_operator.h"

namespace mongo {
namespace tinyjs {

BinaryOperator::BinaryOperator(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node*>* getChildren() {
    std::vector<Node* > children;
    children.push_back(leftChild.get());
    children.push_back(rightChild.get());
    return &children;
}

Node* BinaryOperator::getLeftChild() {
    return leftChild.get();
}

Node* BinaryOperator::getRightChild() {
    return rightChild.get();
}

void BinaryOperator::setLeftChild(std::unique_ptr<Node> node) {
    leftChild = std::move(node);
}

void BinaryOperator::setRightChild(std::unique_ptr<Node> node) {
    rightChild = std::move(node);
}

}
}
