#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class NullLiteralNode : public TerminalNode {
    NullLiteralNode();
    ~NullLiteralNode();
    std::string getName();
    int getValue();
}

}
}
