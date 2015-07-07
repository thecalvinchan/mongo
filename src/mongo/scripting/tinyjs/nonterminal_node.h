#include "mongo/scripting/tinyjs/node.h"

namespace mongo {
namespace tinyjs {

class NonTerminalNode : public Node {
public:
    NonTerminalNode(TokenType type);
    virtual ~NonTerminalNode() = 0;
    //getValue and getChildren are only used for testing
    virtual std::vector<std::unique_ptr<Node> >* getChildren() = 0;
    Value* getValue() const;
    TokenType getType();
private:
    TokenType type;
};

}
}
