#include "mongo/scripting/tinyjs/node.h"
#include "mongo/scripting/tinyjs/ast_parser.h"
#include "mongo/scripting/tinyjs/assignment_operator.h"
#include "mongo/scripting/tinyjs/arithmetic_operator.h"
#include "mongo/scripting/tinyjs/comparison_operator.h"
#include "mongo/scripting/tinyjs/logical_operator.h"
#include "mongo/scripting/tinyjs/unary_operator.h"
#include "mongo/scripting/tinyjs/operand.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
ASTParser::ASTParser(std::vector<Token> tokens) {
    for (size_t i=0; i<tokens.size(); i++) {
        //Value *value = new Value(tokens[i].value);
        switch(tokens[i].type) {
        case thisToken:
            //Node *node = new OperandNode(value, OperandNode::OBJECT);
            break;
        default:
            break;
        }
    }
}

}
