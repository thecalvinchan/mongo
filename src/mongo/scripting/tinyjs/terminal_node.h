#include "mongo/scripting/tinyjs/ast_node.h"

namespace mongo {
namespace tinyjs {

class TerminalNode : public Node {
    TerminalNode();
    virtual ~TerminalNode() = 0;
};

}
}
