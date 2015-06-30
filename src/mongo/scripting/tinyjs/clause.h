#ifndef OPERATOR_NODE_H
#define OPERATOR_NODE_H

#include "mongo/db/pipeline/value.h"
#include <unordered_map>
#include "node.h"
#include "terminal.h"

namespace mongo {

class ClauseNode: public Node {
public:
    static string type = "CLAUSE";
    ClauseNode();
    virtual ~ClauseNode();
};

}

#endif
