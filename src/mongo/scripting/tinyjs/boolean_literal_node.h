#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class BooleanLiteralNode : public TerminalNode {
public:
    BooleanLiteralNode(float val);
    ~BooleanLiteralNode();
    std::string getName();
    bool getValue();
private:
    bool value;
}

BooleanLiteralNode::BooleanLiteralNode(float val) : value(val) {}

std::string BooleanLiteralNode::getName() {
    return value ? "true" : "false";
}

bool BooleanLiteralNode::getValue() {
    return value;
}

}
}

