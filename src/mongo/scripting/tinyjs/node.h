#include "mongo/db/pipeline/value.h"

#ifndef NODE_H
#define NODE_H

namespace mongo {

class Node {
public:
    static Node* createNodeFromNodeType(NodeType type) {
        switch (type) {
            kThisIdentifier,
            kReturnIdentifier,
            kNullIdentifier,
            kUndefinedIdentifier,
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
            kFunctionDec,
            kOpenCurly,
            kCloseCurly, 
            kError,
        }
    }
    static enum NodeType {
        kThisIdentifier,
        kReturnIdentifier,
        kNullIdentifier,
        kUndefinedIdentifier,
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
        kFunctionDec,
        kOpenCurly,
        kCloseCurly, 
        kError,
    };
    Node();
    virtual ~Node();
    virtual Value *returnValue() const;
};

}

#endif
