#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

NonTerminalNode::NonTerminalNode(TokenType type) : type(type) {
}

Value* const NonTerminalNode::getValue() {
    return NULL;
}

TokenType NonTerminalNode::getType() {
    return type;
}

}
}
