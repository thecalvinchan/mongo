#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/bson/bsontypes.h"

namespace mongo {
namespace tinyjs {

TerminalNode::TerminalNode(const NullLabeler&) : _isIdentifierBool(false) {
    _value = Value(BSONNULL);
}
TerminalNode::TerminalNode(const UndefinedLabeler&) : _isIdentifierBool(false) {
    _value = Value(BSONUndefined);
}
TerminalNode::TerminalNode(const int &value) : _isIdentifierBool(false) {
    _value = Value(value);
}
TerminalNode::TerminalNode(const double &value) : _isIdentifierBool(false) {
    _value = Value(value);
}
TerminalNode::TerminalNode(const bool &value) : _isIdentifierBool(false) {
    _value = Value(value);
}
TerminalNode::TerminalNode(const std::string &value) : _isIdentifierBool(false) {
    _value = Value(value);
}
TerminalNode::TerminalNode(const std::string &value, bool identifier) : _isIdentifierBool(identifier) {
    _value = Value(value);
}

std::vector<Node* > TerminalNode::getChildren() {
    return std::vector<Node>();
}

Value* TerminalNode::getValue() const {
    return &_value;
}

std::string TerminalNode::getName() const {
    if (isIdentifier()) {
        return "identifier";
    } else {
        return typeName(_value.getType());
    }
}

bool const TerminalNode::isIdentifier() {
    return _isIdentifierBool;
}

}
}
