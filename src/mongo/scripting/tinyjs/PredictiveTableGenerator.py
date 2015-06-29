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
    "kCloseCurlyBrace"]

grammar = {
    kClause: [[kFunctionKeyword, kOpenParen, kCloseParen, kOpenCurlyBrace, kReturnStatement, kCloseCurlyBrace], 
        [kReturnStatement]], 
    kVariable: [[kIdentifier], 
        [kObject]], 
    kObject: [[kThisIdentifier, kObject], 
        [kIdentifier, kObjectAccessor]]
    kObjectAccessor: [[kPeriod, kIdentifier, kObjectAccessor], 
        []]
}

def firstSingle(token):
    if (token in terminals):
        return [token]
    else:
        res = []
        for rule in grammar[token]:
                for ruleComponent in rule:
                    newTokens = first(ruleComponent)
                    res += newTokens
                    if ("kOptional" not in newTokens):
                        break
        return res

def firstString(tokens):
    res = []
    optionalEverywhere = True
    for token in tokens:
        newTokens = firstSingle(token)
        res += (newTokens - "kOptional")
        if ("kOptional" not in newTokens):
            optionalEverywhere = False
            break
    if optionalEverywhere:
        res.append("kOptional")

def follow(token):
    res = []
    if (token == "kClause"):
        res.append("$")
    for key, value in grammar.iteritems():
        if (value[1] == token):
            if (len(value) > 2):
                newTokens = first(value[2])
                newTokens -= "kOptional"
                res += newTokens
            if ((len(value) > 1) and ("kOptional" in first(value[0])):  
                res += follow(value[0])
    return res

def table():
    table = {}
    for nonterminal in grammar:
        table[nonterminal] = {}
    for A, alpha in grammar.iteritems():
        for a in first(A):
            table[A][a] = {A: alpha} 
        if ("kOptional" in firstArray(alpha)):
            for b in follow(A):
                table[A][b] = {A: alpha}
            if ("$" in follow(A)):
                table[A]["$"] = {A: alpha}
    return table
























