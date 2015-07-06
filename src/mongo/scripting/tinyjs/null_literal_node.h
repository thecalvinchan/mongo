#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class NullLiteralNode : public TerminalNode {
public:
    NullLiteralNode();
    ~NullLiteralNode();
    std::string getName();
    int getValue();
}

std::string NullLiteralNode::getName() {
    return "null";
}

int NullLiteralNode::getValue() {
    return 0;
}

}
}
