#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class FloatLiteralNode : public TerminalNode {
public:
    FloatLiteralNode(float val);
    ~IntegerLiteralNode();
    std::string getName();
    float getValue();
private:
    float value;
}

FloatLiteralNode::FloatLiteralNode(float val) : value(val) {
}

std::string FloatLiteralNode::getName() {
    return std::to_string(value);
}

float FloatLiteralNode::getValue() {
    return value;
}

}
}

