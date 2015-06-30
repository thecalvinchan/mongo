#include <vector>
#include "mongo/scripting/tinyjs/lexer.h"

namespace mongo {
namespace tinyjs {
class ASTParser {
public:
    ASTParser(std::vector<Token> tokens);
    ~ASTParser();
private:
    void parseTokens(std::vector<Token> tokens);
    void nexttoken(void);
    void error(const char msg[]);
    int accept(TokenType t);
    int accept(std::function<void(void)> action);
    int expect(TokenType t);
    void clauseAction();
    void variableAction();
    void objectAction();
    void objectAccessorAction();
    void termAction();
    void arrayElementAction();
    void arrayLiteralAction();
    void arrayTailAction();
    void arrayIndexedAction();
    void factorAction();
    void multiplicativeExpressionAction();
    void multiplicativeOperationAction();
    void arithmeticExpressionAction();
    void arithmeticOperationAction();
    void booleanFactorAction();
    void relationalExpressionAction();
    void relationalOperationAction();
    void booleanExpressionAction();
    void booleanOperationAction();
    void ternaryOperationAction();
    void returnStatementAction();
    void logicalOperationAction();
    void comparisonOperationAction();

    int currentPosition;
    Token currentToken;
    std::vector<Token> tokens;
};

}
}
