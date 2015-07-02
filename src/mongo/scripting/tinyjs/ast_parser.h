#include <vector>
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/node.h"

namespace mongo {
namespace tinyjs {

class ASTParser {
public:
    ASTParser(std::vector<Token> tokens);
    ~ASTParser();
    void printTree();
    std::string traverse();
private:
    void parseTokens(std::vector<Token> tokens);
    void nexttoken(void);
    void error(const char msg[]);

    std::unique_ptr<Node> acceptIf(TokenType t);
    std::unique_ptr<Node> acceptIf(std::function< std::unique_ptr<Node> (void) > action);
    std::unique_ptr<Node> expect(TokenType t);

    std::unique_ptr<Node> clauseAction();
    std::unique_ptr<Node> variableAction();
    std::unique_ptr<Node> objectAction();
    std::unique_ptr<Node> objectAccessorAction();
    std::unique_ptr<Node> termAction();
    std::unique_ptr<Node> arrayElementAction();
    std::unique_ptr<Node> arrayLiteralAction();
    std::unique_ptr<Node> arrayTailAction();
    std::unique_ptr<Node> arrayIndexedAction();
    std::unique_ptr<Node> factorAction();
    std::unique_ptr<Node> multiplicativeExpressionAction();
    std::unique_ptr<Node> multiplicativeOperationAction();
    std::unique_ptr<Node> arithmeticExpressionAction();
    std::unique_ptr<Node> arithmeticOperationAction();
    std::unique_ptr<Node> booleanFactorAction();
    std::unique_ptr<Node> relationalExpressionAction();
    std::unique_ptr<Node> relationalOperationAction();
    std::unique_ptr<Node> booleanExpressionAction();
    std::unique_ptr<Node> booleanOperationAction();
    std::unique_ptr<Node> ternaryOperationAction();
    std::unique_ptr<Node> returnStatementAction();
    std::unique_ptr<Node> logicalOperationAction();
    std::unique_ptr<Node> comparisonOperationAction();

    int currentPosition;
    Token currentToken;
    std::vector<Token> tokens;
    std::unique_ptr<Node> head;
};

}
}
