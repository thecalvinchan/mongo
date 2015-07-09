#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/bson/bsontypes.h"

namespace mongo {
namespace tinyjs {

TerminalNode::TerminalNode(const NullLabeler&) :  _value(Value(BSONNULL)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const UndefinedLabeler&) : _value(Value(BSONUndefined)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const int &value) : _value(Value(value)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const double &value) : _value(Value(value)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const bool &value) : _value(Value(value)), _isIdentifierBool(false) {
    std::cout << "boolean constructor " << std::endl;
}
TerminalNode::TerminalNode(const StringData &value) : _value(Value(value)), _isIdentifierBool(false) {
    std::cout << "string constructor " << std::endl;
}
TerminalNode::TerminalNode(const StringData &value, bool identifier)
    : _value(Value(value)), _isIdentifierBool(identifier) {}

std::vector<Node* > TerminalNode::getChildren() {
    return std::vector<Node*>();
}

const Value* TerminalNode::getValue() const {
    return &_value;
}

std::string TerminalNode::getName() const {
    if 
    return _value.toString();
}

bool TerminalNode::isIdentifier() const {
    return _isIdentifierBool;
}

}
}
