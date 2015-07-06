#include "mongo/scripting/tinyjs/terminal_node.h"

namespace mongo {
namespace tinyjs {

class FloatLiteralNode : public TerminalNode {
public:
    FloatLiteralNode(float val);
    ~IntegerLiteralNode();
    std::string getName();
    float getValue();
private:
    float value;
}

}
}

