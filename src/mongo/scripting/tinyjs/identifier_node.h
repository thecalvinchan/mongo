#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class IdentifierNode : public TerminalNode {
public:
    IdentifierNode(std::string val);
    ~IdentifierNode();
    std::string getName();
    std::string getValue();
private:
    std::string value;
}

IdentifierNode::IdentifierNode(std::string val) : value(val) {
}

std::string IdentifierNode::getName() {
    return getValue();
}

std::string IdentifierNode::getValue() {
    return value;
}

}
}


