#include <vector>
#include "mongo/scripting/tinyjs/node.h"

namespace mongo {

class ASTParser {
public:
    enum TokenType{
        thisToken, returnToken, integerLiteral, floatLiteral, booleanLiteral, 
        stringLiteral, identifier, multiplicativeOp, additiveOp, comparisonOp, logicalOp, 
        endStatement, openParen, closeParen, questionMark, colon, period, openSqBracket, 
        closeSqBracket
    };

    struct Token{
        TokenType type;
        std::string value;
    };
    ASTParser(std::vector<Token> tokens);
    ~ASTParser();
private:
    std::vector<Node*> nodes;
};

}
