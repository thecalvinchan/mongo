#include <vector>
#include "mongo/scripting/tinyjs/node.h"

namespace mongo {

class ASTParser {
public:
    enum ProductionType {
        kThisIdentifier,
        kReturnKeyword,
        kNullLiteral,
        kUndefinedLiteral,
        kFunctionKeyword,
        kIntegerLiteral,
        kFloatLiteral,
        kBooleanLiteral,
        kStringLiteral,
        kIdentifier,
        kAdd,
        kSubtract,
        kMultiply,
        kDivide,
        kTripleEquals,
        kDoubleEquals,
        kLessThan,
        kLessThanEquals,
        kGreaterThan,
        kGreaterThanEquals,
        kNotEquals,
        kDoubleNotEquals,
        kLogicalAnd,
        kLogicalOr,
        kLogicalNot,
        kSemiColon,
        kOpenParen,
        kCloseParen,
        kQuestionMark,
        kColon,
        kPeriod,
        kOpenSquareBracket,
        kCloseSquareBracket,
        kOpenCurlyBrace,
        kCloseCurlyBrace,
        kOptional, //36
    };

    static std::unordered_map<ProductionType, std::unordered_map<ProductionType, ProductionType[]> > ProductionTable (
        {clause, std::unordered_map<ProductionType, ProductionType> (
                {kFunctionDec, [kFunctionDec, kOpenCurly, returnStatement, kCloseCurly]},
                {kReturnIdentifier, [returnStatement]}
            )},
        {object, std::unordered_map<ProductionType, ProductionType> (
                {kIdentifier, [kIdentifier, objectAccessor]},
            )},
        {objectAccessor, std::unordered_map<ProductionType, ProductionType> (
                {kPeriod, [kIdentifier, objectAccessor]},
            )},
    );

    struct Token{
        ProductionType type;
        std::string value;
    };
    ASTParser(std::vector<Token> tokens);
    ~ASTParser();
private:
    std::vector<Node*> nodes;
};

}
