#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/bson/bsontypes.h"

namespace mongo {
namespace tinyjs {

TerminalNode(const NullLabeler&) : isIdentifierBool(false) {
    value = Value(BSONNULL);
}
TerminalNode(const UndefinedLabeler&) : isIdentifierBool(false) {
    value = Value(BSONUndefined);
}
TerminalNode(const int value&) : isIdentifierBool(false) {
    value = Value(value);
}
TerminalNode(const float value&) : isIdentifierBool(false) {
    value = Value(value);
}
TerminalNode(const bool value&) : isIdentifierBool(false) {
    value = Value(value);
}
TerminalNode(const std::string value&) : isIdentifierBool(false) {
    value = Value(value);
}
TerminalNode(const std::string value&, bool identifier) : isIdentifierBool(identifier) {
    value = Value(value);
}

std::vector<Node* > TerminalNode::getChildren() {
    return NULL;
}

Value* TerminalNode::getValue() const {
    return &value;
}

std::string TerminalNode::getName() const {
    if (isIdentifier()) {
        return "identifier";
    } else {
        return typeName(value.getType());
    }
}

bool const TerminalNode::isIdentifier() {
    return isIdentifierBool;
}

}
}
