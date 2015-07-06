#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class StringLiteralNode : public TerminalNode {
public:
    StringLiteralNode(std::string val);
    ~StringLiteralNode();
    std::string getName();
    std::string getValue();
private:
    std::string value;
}

}
}

