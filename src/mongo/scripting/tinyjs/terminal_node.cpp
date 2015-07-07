#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/bson/bsontypes.h"

namespace mongo {
namespace tinyjs {

TerminalNode(const NullLabeler&) : isIdentifier(false) {
    value = Value(BSONNULL);
}
TerminalNode(const UndefinedLabeler&) : isIdentifier(false) {
    value = Value(BSONUndefined);
}
TerminalNode(const int value&) : isIdentifier(false) {
    value = Value(value);
}
TerminalNode(const float value&) : isIdentifier(false) {
    value = Value(value);
}
TerminalNode(const bool value&) : isIdentifier(false) {
    value = Value(value);
}
TerminalNode(const std::string value&) : isIdentifier(false) {
    value = Value(value);
}
TerminalNode(const std::string value&, bool identifier) : isIdentifier(identifier) {
    value = Value(value);
}

std::vector<std::unique_ptr<Node> >* TerminalNode::getChildren() {
    return NULL;
}

Value* const TerminalNode::getValue() {
    return &value;
}

std::string const TerminalNode::getName() {
    if (isIdentifier()) {
        return "identifier";
    } else {
        return typeName(value.getType());
    }
}

bool const TerminalNode::isIdentifier() {
    return isIdentifier;
}

}
}
