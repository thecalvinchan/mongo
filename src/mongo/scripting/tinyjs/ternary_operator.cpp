#include "mongo/scripting/tinyjs/ternary_operator.h"

namespace mongo {
namespace tinyjs {

TernaryOperator::TernaryOperator(TokenType type) : NonTerminalNode(type) {
}

std::vector<Node*> TernaryOperator::getChildren() {
    std::vector<Node* > children;
    children.push_back(leftChild.get());
    children.push_back(middleChild.get());
    children.push_back(rightChild.get());
    return children;
}

Node* TernaryOperator::getLeftChild() {
    return leftChild.get();
}

Node* TernaryOperator::getMiddleChild() {
    return middleChild.get();
}

Node* TernaryOperator::getRightChild() {
    return rightChild.get();
}

void BinaryOperator::setLeftChild(std::unique_ptr<Node> node) {
    leftChild = std::move(node);
}

void BinaryOperator::setMiddleChild(std::unique_ptr<Node> node) {
    middleChild = std::move(node);
}

void BinaryOperator::setRightChild(std::unique_ptr<Node> node) {
    rightChild = std::move(node);
}

}
}
