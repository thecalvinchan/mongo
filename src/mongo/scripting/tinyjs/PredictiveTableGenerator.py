grammar = {
    "kClause": [
        ["kFunctionKeyword", " kOpenParen", " kCloseParen", " kOpenCurlyBrace", " kReturnStatement", " kCloseCurlyBrace"],
        ["kReturnStatement"]], 
    "kVariable": [
        ["kIdentifier"], 
        ["kObject"]],
    "kObject": [
        ["kThisIdentifier",  "kObjectAccessor"],
        ["kIdentifier", "kObjectAccessor"]],
    "kObjectAccessor": [
        ["kPeriod", "kIdentifier", "kObjectAccessor"],
        ["kOpenSquareBracket", "kIntegerLiteral", "kCloseSquareBracket", "kObjectAccessor"],
        ["kOpenSquareBracket", "kStringLiteral", "kCloseSquareBracket", "kObjectAccessor"],
        ["kOpenSquareBracket", "kIdentifier", "kCloseSquareBracket", "kObjectAccessor"],
        ["kOpenSquareBracket", "kArithmeticExpression", "kCloseSquareBracket", "kObjectAccessor"],
        ["kOptional"]],
    "kTerm": [
        ["kIntegerLiteral"],
        ["kFloatLiteral"],
        ["kStringLiteral"],
        ["kVariable"],
        ["kBooleanLiteral"]],
    "kArrayElement": [
        ["kTerm"],
        ["kArithmeticExpression"],
        ["kBooleanExpression"]],
    "kArrayLiteral": [
        ["kOpenSquareBracket", "kCloseSquareBracket"],
        ["kOpenSquareBracket", "kArrayElement", "kArrayTail", "kCloseSquareBracket"]],
    "kArrayTail": [
        ["kComma", "kArrayElement", "kArrayTail"],
        ["kOptional"]],
    "kArrayIndexed": [
        ["kIdentifier", "kOpenSquareBracket", "kIntegerLiteral", "kCloseSquareBracket"],
        ["kIdentifier", "kOpenSquareBracket", "kIdentifier", "kCloseSquareBracket"],
        ["kIdentifier", "kOpenSquareBracket", "kArithmeticExpression", "kCloseSquareBracket"]],
    "kFactor": [
        ["kTerm"],
        ["kOpenParen", "kArithmeticExpression", "kCloseParen"]],
    
    "kMultiplicativeExpression": [ 
        ["kFactor", "kMultiplicativeOperation"]],
    "kMultiplicativeOperation": [
        ["kMultiply", "kFactor", "kMultiplicativeOperation"],
        ["kDivide", "kFactor", "kMultiplicativeOperation"],
        [ "kOptional"]],
    "kArithmeticExpression": [ 
        ["kMultiplicativeExpression", "kArithmeticOperation"]],
    "kArithmeticOperation": [
        ["kAdd", "kMultiplicativeExpression", "kArithmeticOperation"],
        ["kSubtract", "kMultiplicativeExpression", "kArithmeticOperation"],
        ["kOptional"]],
    "kBooleanFactor": [
        ["kArithmeticExpression"],
        ["kOpenParen", "kBooleanExpression", "kCloseParen"]],
    "kRelationalExpression": [
        ["kBooleanFactor", "kRelationalOperation"]], 
    "kRelationalOperation": [
        ["kComparisonOp", "kBooleanFactor", "kRelationalOperation"]
        ["kOptional"]],
    "kBooleanExpression": [ 
        ["kRelationalExpression", "kBooleanOperation"]],
    "kBooleanOperation": [
        ["kLogicalOp", "kRelationalExpression", "kBooleanOperation"],
        ["kOptional"]],
    "kTernaryOp": [ 
        ["kBooleanExpression", "kQuestionMark", "kBooleanExpression", "kColon", "kBooleanExpression"]]
    "kReturnStatement": [
        ["kReturnKeyword", "kBooleanExpression", "kSemiColon", ]]
}
