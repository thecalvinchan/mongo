#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class UndefinedLiteralNode : public TerminalNode {
    UndefinedLiteralNode();
    ~UndefinedLiteralNode();
    std::string getName();
    int getValue();
}

std::string UndefinedLiteralNode::getName() {
    return "undefined";
}

int NullLiteralNode::getValue() {
    return 0;
}

}
}
