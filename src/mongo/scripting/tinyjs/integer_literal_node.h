#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class IntegerLiteralNode : public TerminalNode {
public:
    Integer(int val);
    ~IntegerLiteralNode();
    std::string getName();
    int getValue();
private:
    int value;
}

IntegerLiteralNode::IntegerLiteralNode(int val) : value(val) {
}

std::string IntegerLiteralNode::getName() {
    return std::to_string(value);
}

int IntegerLiteralNode::getValue() {
    return value;
}

}
}
