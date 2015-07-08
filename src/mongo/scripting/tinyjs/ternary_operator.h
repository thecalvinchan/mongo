#pragma once

#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

class TernaryOperator : public NonTerminalNode {
public:
    TernaryOperator(TokenType type);
    ~TernaryOperator() {}
    std::vector<Node*> getChildren();
    Node* getLeftChild();
    Node* getMiddleChild();
    Node* getRightChild();
    void setLeftChild(std::unique_ptr<Node> node);
    void setMiddleChild(std::unique_ptr<Node> node);
    void setRightChild(std::unique_ptr<Node> node);
private:
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> middleChild;
    std::unique_ptr<Node> rightChild;
};

}
}
