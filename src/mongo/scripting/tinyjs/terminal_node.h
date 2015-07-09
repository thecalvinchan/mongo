#pragma once

#include "mongo/scripting/tinyjs/node.h"
#include "mongo/db/pipeline/value.h"
#include "mongo/base/string_data.h"

namespace mongo {
namespace tinyjs {

class TerminalNode : public Node {
public:
    TerminalNode(const NullLabeler &value);
    TerminalNode(const UndefinedLabeler &value);
    TerminalNode(const int &value);
    TerminalNode(const double &value);
    TerminalNode(const bool &value);
    TerminalNode(const StringData &value);
    TerminalNode(const StringData &value, bool identifier);
    ~TerminalNode() {}
    std::vector<Node* > getChildren(); //TODO const reference
    const Value* getValue() const;
    StringData getName() const;
    bool isIdentifier() const;
private:
    Value _value;
    bool _isIdentifierBool;
};

}
}
