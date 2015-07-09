#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/bson/bsontypes.h"

namespace mongo {
namespace tinyjs {

TerminalNode::TerminalNode(const NullLabeler&) :  _value(Value(BSONNULL)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const UndefinedLabeler&) : _value(Value(BSONUndefined)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const int &value) : _value(Value(value)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const double &value) : _value(Value(value)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const bool &value) : _value(Value(value)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const StringData &value) : _value(Value(value)), _isIdentifierBool(false) {}
TerminalNode::TerminalNode(const StringData &value, bool identifier)
    : _value(Value(value)), _isIdentifierBool(identifier) {}

std::vector<Node* > TerminalNode::getChildren() {
    return std::vector<Node*>();
}

const Value* TerminalNode::getValue() const {
    return &_value;
}

StringData TerminalNode::getName() const {
    std::string res = _value.toString();
    //TODO: figure out better way of dealing with the extra quotes around stringdata values
    if (res.front() == '"') {
        res.erase(0, 1); // erase the first character
        if (res.back() == '"') {
            res.erase(res.size() - 1); // erase the last character
        }
    }
    return StringData(res);
}

bool TerminalNode::isIdentifier() const {
    return _isIdentifierBool;
}

}
}
