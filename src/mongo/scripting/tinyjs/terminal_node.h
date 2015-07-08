#pragma once

#include "mongo/scripting/tinyjs/node.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace tinyjs {

class TerminalNode : public Node {
public:
    TerminalNode(const NullLabeler &value);
    TerminalNode(const UndefinedLabeler &value);
    TerminalNode(const int &value);
    TerminalNode(const double &value);
    TerminalNode(const bool &value);
    TerminalNode(const std::string &value);
    TerminalNode(const std::string &value, bool identifier);
    ~TerminalNode() {}
    std::vector<Node* > getChildren(); //TODO const reference
    const Value* getValue() const;
    std::string getName() const;
    bool isIdentifier() const;
private:
    Value _value;
    bool _isIdentifierBool;
};

}
}
