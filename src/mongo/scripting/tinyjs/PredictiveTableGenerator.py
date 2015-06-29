import pprint

terminals = [    
    "kThisIdentifier",
    "kReturnKeyword",
    "kNullLiteral",
    "kUndefinedLiteral",
    "kFunctionKeyword",
    "kIntegerLiteral",
    "kFloatLiteral",
    "kBooleanLiteral",
    "kStringLiteral",
    "kIdentifier",
    "kAdd",
    "kSubtract",
    "kMultiply",
    "kDivide",
    "kTripleEquals",
    "kDoubleEquals",
    "kLessThan",
    "kLessThanEquals",
    "kGreaterThan",
    "kGreaterThanEquals",
    "kNotEquals",
    "kDoubleNotEquals",
    "kLogicalAnd",
    "kLogicalOr",
    "kLogicalNot",
    "kSemiColon",
    "kOpenParen",
    "kCloseParen",
    "kQuestionMark",
    "kColon",
    "kPeriod",
    "kComma",
    "kOpenSquareBracket",
    "kCloseSquareBracket",
    "kOpenCurlyBrace",
    "kCloseCurlyBrace", 
    "kOptional"]

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
        ["kComparisonOp", "kBooleanFactor", "kRelationalOperation"],
        ["kOptional"]],
    "kBooleanExpression": [ 
        ["kRelationalExpression", "kBooleanOperation"]],
    "kBooleanOperation": [
        ["kLogicalOp", "kRelationalExpression", "kBooleanOperation"],
        ["kOptional"]],
    "kTernaryOp": [ 
        ["kBooleanExpression", "kQuestionMark", "kBooleanExpression", "kColon", "kBooleanExpression"]],
    "kReturnStatement": [
        ["kReturnKeyword", "kBooleanExpression", "kSemiColon", ]], 
    "kLogicalOp": [
        ["kLogicalAnd"],
        ["kLogicalOr"]],
    "kComparisonOp": [
        ["kTripleEquals"], 
        ["kDoubleEquals"], 
        ["kGreaterThan"], 
        ["kGreaterThanEquals"], 
        ["kLessThan"], 
        ["kLessThanEquals"], 
        ["kNotEquals"], 
        ["kDoubleNotEquals"]]
}

def firstSingle(token):
    if (token in terminals):
        return [token]
    else:
        res = []
        for rule in grammar[token]:
            res += firstString(rule)
        return res

def firstString(tokens):
    res = []
    optionalEverywhere = True
    for token in tokens:
        newTokens = firstSingle(token)
        if ("kOptional" in newTokens):
            newTokens.remove("kOptional")
        res += newTokens
        if ("kOptional" not in firstSingle(token)):
            optionalEverywhere = False
            break
    if optionalEverywhere:
        res.append("kOptional")
    return res

def follow(token):
    res = []
    if (token == "kClause"):
        res.append("$")
    for key, valueList in grammar.iteritems():
        for value in valueList:
            for i in range(1,len(value)):
                if (value[i] == token):
                    if (len(value) > i + 1):
                        newTokens = firstString(value[(i+1):])
                        if ("kOptional" in newTokens):
                            newTokens.remove("kOptional")
                        res += newTokens
                    if ((i == (len(value) - 1))
                      or ("kOptional" in firstString(value[(i+1):]))): 
                        if (key != token): 
                            res += follow(key)
    return res

def table():
    table = {}
    for nonterminal in grammar:
        table[nonterminal] = {}
    for A, alphaList in grammar.iteritems():
        for alpha in alphaList:
            for a in firstString(alpha):
                table[A][a] = alpha
            if ("kOptional" in firstString(alpha)):
                for b in follow(A):
                    table[A][b] = alpha
                if ("$" in follow(A)):
                    table[A]["$"] = alpha
    return table

pp = pprint.PrettyPrinter(indent=4)
pp.pprint(table())