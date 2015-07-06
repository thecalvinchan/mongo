#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class IntegerLiteralNode : public TerminalNode {
public:
    Integer(int val);
    ~IntegerLiteralNode();
    std::string getName();
    int getValue();
private:
    int value;
}

}
}
