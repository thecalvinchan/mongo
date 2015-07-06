#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

class TernaryOperator : public NonTerminalNode {
public:
    TernaryOperator(TokenType type);
    ~TernaryOperator();
    std::vector<Node* >* getChildren();
    Node* getLeftChild();
    Node* getMiddleChild();
    Node* getRightChild();
    void setLeftChild();
    void setMiddleChild();
    void setRightChild();
private:
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> middleChild;
    std::unique_ptr<Node> rightChild;
};

}
}
