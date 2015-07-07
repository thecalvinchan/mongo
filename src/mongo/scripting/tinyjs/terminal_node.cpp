#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/bson/bsontypes.h"

namespace mongo {
namespace tinyjs {

TerminalNode::TerminalNode(Value val) {
    value = val;
}

std::vector<std::unique_ptr<Node> >* TerminalNode::getChildren() {
    return NULL;
}

Value* const TerminalNode::getValue() {
    return &value;
}

std::string const TerminalNode::getName() {
    typeName(value.getType());
}

}
}
