#include "mongo/scripting/tinyjs/nonterminal_node.h"

namespace mongo {
namespace tinyjs {

NonTerminalNode::NonTerminalNode(TokenType type) : _type(type) {
    _values[TokenType::kPeriod] = ".";

    _values = {{TokenType::kPeriod, "."},
               {TokenType::kOpenSquareBracket, "["},
               {TokenType::kMultiply, "*"},
               {TokenType::kDivide, "/"},
               {TokenType::kAdd, "+"},
               {TokenType::kSubtract, "-"},
               {TokenType::kTripleEquals, "==="},
               {TokenType::kDoubleEquals, "=="},
               {TokenType::kGreaterThan, ">"},
               {TokenType::kGreaterThanEquals, ">="},
               {TokenType::kLessThan, "<"},
               {TokenType::kLessThanEquals, "<="},
               {TokenType::kLogicalAnd, "&&"},
               {TokenType::kLogicalOr, "||"},
               {TokenType::kQuestionMark, "?"},
               {TokenType::kReturnKeyword, "return"}};
}

Value* NonTerminalNode::getValue() const {
    return NULL;
}

std::string NonTerminalNode::getName() const {
    return _values[_type];
}

TokenType NonTerminalNode::getType() {
    return _type;
}

}
}
