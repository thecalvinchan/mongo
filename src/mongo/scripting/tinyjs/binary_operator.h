#pragma once

#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

class BinaryOperator : public NonTerminalNode {
public:
    BinaryOperator(TokenType type);
    ~BinaryOperator() {}
    std::vector<Node*> getChildren();
    Node* getLeftChild();
    Node* getRightChild();
    void setLeftChild(std::unique_ptr<Node>);
    void setRightChild(std::unique_ptr<Node>);
private:
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> rightChild;
};

}
}
