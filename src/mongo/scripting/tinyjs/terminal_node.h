#include "mongo/scripting/tinyjs/node.h"

namespace mongo {
namespace tinyjs {

class TerminalNode : public Node {
public:
    TerminalNode(Value val);
    ~TerminalNode();
    std::vector<std::unique_ptr<Node> >* getChildren();
    Value* getValue() const;
    std::string getName() const;
private:
    Value value;
};

}
}
