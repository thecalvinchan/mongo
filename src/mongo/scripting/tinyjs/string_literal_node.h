#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class StringLiteralNode : public TerminalNode {
public:
    StringLiteralNode(std::string val);
    ~StringLiteralNode();
    std::string getName();
    std::string getValue();
private:
    std::string value;
}

StringLiteralNode::StringLiteralNode(std::string val) : value(val) {
}

std::string StringLiteralNode::getName() {
    return value;
}

std::string StringLiteralNode::getValue() {
    return value;
}

}
}

