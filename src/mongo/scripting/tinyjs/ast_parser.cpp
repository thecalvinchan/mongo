#include "mongo/scripting/tinyjs/node.h"
#include "mongo/scripting/tinyjs/ast_parser.h"
#include "mongo/scripting/tinyjs/assignment_operator.h"
#include "mongo/scripting/tinyjs/arithmetic_operator.h"
#include "mongo/scripting/tinyjs/comparison_operator.h"
#include "mongo/scripting/tinyjs/logical_operator.h"
#include "mongo/scripting/tinyjs/unary_operator.h"
#include "mongo/scripting/tinyjs/operand.h"
#include "mongo/db/pipeline/value.h"
#include <stack>

namespace mongo {

Node* parseTokens(std::vector<Token>::iterator it,
                  std::vector<Token>::iterator end,
                  TokenType stopToken = null);

ASTParser::ASTParser(std::vector<Token> tokens) {
    Node* head = parseTokens(tokens.begin(), tokens.end());
}

bool checkMatch (std::vector<Token> tokens, NodeType startProduction) {
    std::vector<Token>::iterator ip tokenPointer = tokens.begin();
    std::stack<ProductionType> productionStack;
    productionStack.push_back(CLAUSE);
    while (!productionStack.empty()) {
        ProductionType nextProd = productionStack.top();
        if (nextProd == ip->type) {
            productionStack.pop();
            ip++;
        } else if (nextProd == kOptional) {
            productionStack.pop();
        } else if (isTerminal(nextProd)) { 
            // TODO: throw exception
        } else {
            ProductionType productions[] = ProductionTable.at(nextProd).at(ip->type);
            // throws out_of_range exception if not found
            // this means no entry in production table
            // TODO: build tree and node
            productionStack.pop();
            for (int i = productions.size()-1; i>=0; i--) {
                productionStack.push(productions[i]);
            }
        }
    }
}

bool isTerminal(ProductionType production) {
    if (production < 38) {
        return true;
    }
}
