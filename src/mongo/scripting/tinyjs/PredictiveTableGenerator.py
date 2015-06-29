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