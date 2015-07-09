#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

NonTerminalNode::NonTerminalNode(TokenType type) : _type(type) {}

const Value* NonTerminalNode::getValue() const {
    return NULL;
}

StringData NonTerminalNode::getName() const {
    return StringData(names[(int)_type]);
}

TokenType NonTerminalNode::getType() {
    return _type;
}

const std::string NonTerminalNode::names[] = {
    "this",
    "return",
    "NULL",
    "undefined",
    "function",
    "integer",
    "float",
    "bool",
    "string",
    "_id",
    "+",
    "-",
    "*",
    "/",
    "===",
    "==",
    "<",
    "<=",
    ">",
    ">=",
    "!=",
    "!==",
    "&&",
    "||",
    "!",
    ";",
    "(",
    ")",
    "?",
    ":",
    ".",
    ",",
    "[",
    "]",
    "{",
    "}"
};

}
}
