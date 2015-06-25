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

ASTParser::ASTParser(std::vector<Token> tokens) {
    Node *head = parseTokens(tokens.begin(), tokens.end());
}

void balanceStacks(Node *newOpNode, std::stack<Node *> *operandStack, std::stack<Node *> *operatorStack) {
    while (operatorStack->size() > 0 && operatorStack->top().getBinaryOpType() >= newOpNode->getBinaryOpType()) {
        Node *op = operators.pop();
        Node *rChild = operands.pop();
        Node *lChild = operands.pop();
        op->setLChild(lChild);
        op->setRChild(rChild);
        operandStack->push(op);
    }
    operatorStack->push(newOpNode);
}

Node * parseTokens(std::vector<Token>::iterator it, std::vector<Token>::iterator end, TokenType stopToken = null) {
    std::stack<Node *> operands;
    std::stack<Node *> operators;
    for (; *it != stopToken || it != end; it++) {
        Value *value = new Value(tokens[i].value);
        Node *node;
        switch(tokens[i].type) {
            case integerLiteral: {
                node = new OperandNode(value, OperandNode::INTEGER_L);
                operands.push(node);
                break;
            }
            case floatLiteral: {
                node = new OperandNode(value, OperandNode::FLOAT_L);
                operands.push(node);
                break;
            }
            case booleanLiteral: {
                node = new OperandNode(value, OperandNode::BOOLEAN_L);
                operands.push(node);
                break;
            }
            case stringLiteral: {
                node = new OperandNode(value, OperandNode::STRING_L);
                operands.push(node);
                break;
            }
            case thisToken: {
                node = new OperandNode(value, OperandNode::OBJECT);
                operands.push(node);
                break;
            }
            case identifier: {
                node = new OperandNode(value, OperandNode::IDENTIFIER);
                operands.push(node);
                break;
            }
            case returnToken: {
                node = new UnaryOperator(value, UnaryOperator::RETURN);
                break;
            }
            case multiplicativeOp: {
                node = new BinaryOperator(value, BinaryOperator::MULTIPLICATIVE);
                balanceStacks(node, operands, operators);
                break;
            }
            case additiveOp: {
                node = new BinaryOperator(value, BinaryOperator::ADDITIVE);
                balanceStacks(node, operands, operators);
                break;
            }
            case comparisonOp: {
                node = new BinaryOperator(value, BinaryOperator::COMPARISON);
                balanceStacks(node, operands, operators);
                break;
            }
            case logicalOp: {
                node = new BinaryOperator(value, BinaryOperator::LOGICAL);
                balanceStacks(node, operands, operators);
                break;
            }
            case endStatement: {
                // TODO
                break;
            }
            case questionMark: {
                // TODO
                Node *subTreeHead = parseTokens(it++, end, colon);
                Node *subTreeHead = parseTokens(it++, end, endStatement);
                break;
            }
            case colon: {
                // TODO
                break;
            }
            case period: {
                node = new BinaryOperator(value, BinaryOperator::OBJ_ACCESSOR);
                balanceStacks(node, operands, operators);
                break;
            }
            case openParen: {
                Node *subTreeHead = parseTokens(it++, end, closeParen);
                operands.push(subTreeHead);
            }
            case openSqBracket: {
                Node *subTreeHead = parseTokens(it++, end, closeSqBracket);
                node = new BinaryOperator("[ ]", BinaryOperator::OBJ_ACCESSOR);
                balanceStacks(node, operands, operators);
                operands.push(subTreeHead);
            }
            case closeParen:
            case closeSqBracket:
            default:
                // Malformed Expr
                break;
        }
    }
    while (operators.size() > 0) {
    }
    if (it == end && stopToken != null) {
        // Malformed Expr
    } else {
    }
}
