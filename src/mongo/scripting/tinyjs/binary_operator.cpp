/**
 * Copyright (C) 2015 MongoDB Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the copyright holders give permission to link the
 * conditions as described in each individual source file and distribute
 * linked combinations including the program with the OpenSSL library. You
 * must comply with the GNU Affero General Public License in all respects
 * for all of the code used other than as permitted herein. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you do not
 * wish to do so, delete this exception statement from your version. If you
 * delete this exception statement from all source files in the program,
 * then also delete it in the license file.
 */

#include "mongo/platform/basic.h"
#include "mongo/bson/bsontypes.h"
#include "mongo/base/checked_cast.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/scripting/tinyjs/binary_operator.h"

namespace mongo {
namespace tinyjs {

BinaryOperator::BinaryOperator(TokenType type) : NonTerminalNode(type) {}

std::vector<Node*> BinaryOperator::getChildren() const {
    std::vector<Node*> children;
    children.push_back(_leftChild.get());
    children.push_back(_rightChild.get());
    return children;
}

Node* BinaryOperator::getLeftChild() const {
    return _leftChild.get();
}

Node* BinaryOperator::getRightChild() const {
    return _rightChild.get();
}

void BinaryOperator::setLeftChild(std::unique_ptr<Node> node) {
    _leftChild = std::move(node);
}

void BinaryOperator::setRightChild(std::unique_ptr<Node> node) {
    _rightChild = std::move(node);
}

const Value BinaryOperator::evaluate(Scope* scope) const {
    switch (this->getType()) {
        case TokenType::kPeriod:
            return evaluateObjectAccessor(scope);
            break;
        case TokenType::kOpenSquareBracket:
            break;
        case TokenType::kMultiply:
            return evaluateMultiply(scope);
        case TokenType::kDivide:
            return evaluateDivide(scope);
        case TokenType::kAdd:
            return evaluateAdd(scope);
        case TokenType::kSubtract:
            return evaluateSubtract(scope);
        case TokenType::kTripleEquals:
            break;
        case TokenType::kDoubleEquals:
            break;
        case TokenType::kGreaterThan:
            return evaluateGreaterThan(scope);
        case TokenType::kGreaterThanEquals:
            return evaluateGreaterThanEquals(scope);
        case TokenType::kLessThan:
            return evaluateLessThan(scope);
        case TokenType::kLessThanEquals:
            return evaluateLessThanEquals(scope);
        case TokenType::kNotEquals:
            break;
        case TokenType::kDoubleNotEquals:
            break;
        case TokenType::kLogicalAnd:
            break;
        case TokenType::kLogicalOr:
            break;
        default:
            break;
    }

    return Value();
}

const Value BinaryOperator::evaluateMultiply(Scope* scope) const {
    Value leftValue = this->getLeftChild()->evaluate(scope);
    Value rightValue = this->getRightChild()->evaluate(scope);
    if (!countsAsNumber(leftValue) || !countsAsNumber(rightValue)) {
        throw std::runtime_error("NaN");
    }

    if (leftValue.getType() == String) {
        if (makeString(leftValue) == "Infinity") {
            if (isZero(rightValue)) {
                throw std::runtime_error("NaN");
            }
            return (isNegative(rightValue) ? Value("-Infinity") : Value("Infinity"));
        } else if (makeString(leftValue) == "-Infinity") {
            if (isZero(rightValue)) {
                throw std::runtime_error("NaN");
            }
            return (isNegative(leftValue) ? Value("Infinity") : Value("-Infinity"));
        } else {
            throw std::runtime_error(
                "NaN");  // TODO should be unreachable, except maybe type errors
        }
    } else if (rightValue.getType() == String) {
        std::cout << makeString(rightValue) << std::endl;
        if (makeString(rightValue) == "Infinity") {
            if (isZero(leftValue)) {
                throw std::runtime_error("NaN");
            }
            return Value("Infinity");
        } else if (rightValue.toString() == "-Infinity") {
            if (isZero(leftValue)) {
                throw std::runtime_error("NaN");
            }
            return Value("-Infinity");
        } else {
            throw std::runtime_error("NaN");  // TODO should be unreachable
        }
    }

    leftValue = makeNumeric(leftValue);
    rightValue = makeNumeric(rightValue);

    if (leftValue.getType() == NumberDouble) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getDouble() * rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getDouble() * rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getDouble() * rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberInt) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getInt() * rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getInt() * rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getInt() * rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberLong) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getLong() * rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getLong() * rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getLong() * rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else {
        throw std::runtime_error("NaN");
    }
}


const Value BinaryOperator::evaluateDivide(Scope* scope) const {
    Value leftValue = this->getLeftChild()->evaluate(scope);
    Value rightValue = this->getRightChild()->evaluate(scope);

    if (!countsAsNumber(leftValue) || !countsAsNumber(rightValue)) {
        std::cout << "don't both count as numbers" << std::endl;
        throw std::runtime_error("NaN");
    }

    if (leftValue.getType() == String) {
        if (makeString(leftValue) == "Infinity") {
            if ((rightValue.getType() == String && ((makeString(rightValue) == "Infinity") ||
                                                    (makeString(rightValue) == "-Infinity")))) {
                throw std::runtime_error("NaN");
            }
            return (isNegative(rightValue) ? Value("-Infinity") : Value("Infinity"));
        } else if (makeString(leftValue) == "-Infinity") {
            if ((rightValue.getType() == String && ((makeString(rightValue) == "Infinity") ||
                                                    (makeString(rightValue) == "-Infinity")))) {
                throw std::runtime_error("NaN");
            }
            return (isNegative(leftValue) ? Value("Infinity") : Value("-Infinity"));
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (rightValue.getType() == String) {
        std::cout << makeString(rightValue) << std::endl;
        // TODO return -0 when one of the 2 is negative
        if (makeString(rightValue) == "Infinity") {
            return Value(0);
        } else if (rightValue.toString() == "-Infinity") {
            return Value(0);
        } else {
            throw std::runtime_error("NaN");  // TODO should be unreachable
        }
    }

    leftValue = makeNumeric(leftValue);
    rightValue = makeNumeric(rightValue);

    if (isZero(rightValue)) {
        if (isZero(leftValue)) {
            std::cout << "0/0 " << std::endl;
            throw std::runtime_error("NaN");
        } else {
            std::cout << "right value is 0 " << std::endl;
            return (isNegative(leftValue) ? Value("-Infinity") : Value("Infinity"));
        }
    } else if (leftValue.getType() == NumberDouble) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getDouble() / rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getDouble() / rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getDouble() / rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberInt) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getInt() / rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getInt() / rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getInt() / rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }

    } else if (leftValue.getType() == NumberLong) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getLong() / rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getLong() / rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getLong() / rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else {
        throw std::runtime_error("NaN");
    }
}

const Value BinaryOperator::evaluateAdd(Scope* scope) const {
    Value leftValue = this->getLeftChild()->evaluate(scope);
    Value rightValue = this->getRightChild()->evaluate(scope);

    if (isString(leftValue)) {
        return Value(makeString(leftValue) + makeString(rightValue));
    } else if (isString(rightValue)) {
        return Value(makeString(leftValue) + makeString(rightValue));
    } else if ((leftValue.getType() == Array) || (rightValue.getType() == Array)) {
        return Value(makeString(leftValue) + makeString(rightValue));
    }

    if (!countsAsNumber(leftValue) || !countsAsNumber(rightValue)) {
        throw std::runtime_error("NaN");
    }

    if (leftValue.getType() == String) {
        if (makeString(leftValue) == "Infinity") {
            if ((rightValue.getType() == String) && (makeString(rightValue) == "-Infinity")) {
                throw std::runtime_error("NaN");
            } else {
                return Value("Infinity");
            }
        } else {
            massert(1,
                    "should not reach this point",
                    makeString(leftValue) == "-Infinity");  // is this good style?
            if ((rightValue.getType() == String) && (makeString(rightValue) == "Infinity")) {
                throw std::runtime_error("NaN");
            } else {
                return Value("-Infinity");
            }
        }
    } else if (rightValue.getType() == String) {
        std::cout << makeString(rightValue) << std::endl;
        if (makeString(rightValue) == "Infinity") {
            return Value("Infinity");
        } else if (rightValue.toString() == "-Infinity") {
            return Value("-Infinity");
        } else {
            throw std::runtime_error("Should not reach here");  // TODO should be unreachable
        }
    }

    leftValue = makeNumeric(leftValue);
    rightValue = makeNumeric(rightValue);

    if (leftValue.getType() == NumberDouble) {
        // TODO: refactor to share code with multiplication?
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getDouble() + rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getDouble() + rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getDouble() + rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberInt) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getInt() + rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getInt() + rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getInt() + rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberLong) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getLong() + rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getLong() + rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getLong() + rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else {
        throw std::runtime_error("NaN");
    }
}

const Value BinaryOperator::evaluateSubtract(Scope* scope) const {
    Value leftValue = this->getLeftChild()->evaluate(scope);
    Value rightValue = this->getRightChild()->evaluate(scope);

    if (!countsAsNumber(leftValue) || !countsAsNumber(rightValue)) {
        throw std::runtime_error("NaN");
    }

    if (leftValue.getType() == String) {
        if (makeString(leftValue) == "Infinity") {
            if ((rightValue.getType() == String) && (makeString(rightValue) == "Infinity")) {
                throw std::runtime_error("NaN");
            } else {
                return Value("Infinity");
            }
        } else {
            massert(2,
                    "should not reach this point",
                    makeString(leftValue) == "-Infinity");  // is this good style?
            if ((rightValue.getType() == String) && (makeString(rightValue) == "-Infinity")) {
                throw std::runtime_error("NaN");
            } else {
                return Value("-Infinity");
            }
        }
    } else if (rightValue.getType() == String) {
        std::cout << makeString(rightValue) << std::endl;
        if (makeString(rightValue) == "Infinity") {
            return Value("-Infinity");
        } else if (rightValue.toString() == "-Infinity") {
            return Value("Infinity");
        } else {
            throw std::runtime_error("Should not reach here");  // TODO should be unreachable
        }
    }

    leftValue = makeNumeric(leftValue);
    rightValue = makeNumeric(rightValue);

    if (leftValue.getType() == NumberDouble) {
        // TODO: refactor to share code with multiplication?
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getDouble() - rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getDouble() - rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getDouble() - rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberInt) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getInt() - rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getInt() - rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getInt() - rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else if (leftValue.getType() == NumberLong) {
        if (rightValue.getType() == NumberDouble) {
            return Value(leftValue.getLong() - rightValue.getDouble());
        } else if (rightValue.getType() == NumberInt) {
            return Value(leftValue.getLong() - rightValue.getInt());
        } else if (rightValue.getType() == NumberLong) {
            return Value(leftValue.getLong() - rightValue.getLong());
        } else {
            throw std::runtime_error("NaN");
        }
    } else {
        throw std::runtime_error("NaN");
    }
}

bool isZero(Value value) {
    return (((value.getType() == NumberInt) && (value.getInt() == 0)) ||
            ((value.getType() == NumberDouble) && (value.getDouble() == 0)) ||
            ((value.getType() == NumberLong) && (value.getLong() == 0)) ||
            (value.getType() == jstNULL) ||
            ((value.getType() == Bool) && value.getBool() == false));
}

bool isNegative(Value value) {
    return ((value.numeric() && (value.coerceToInt() < 0)) ||
            ((value.getType() == String) && makeString(value) == "-Infinity"));
}

std::string stripDoubleQuotes(std::string s) {
    if (((s.front() == '"') && (s.back() == '"')) || ((s.front() == '\'') && (s.back() == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
    if (((s.front() == '"') && (s.back() == '"')) || ((s.front() == '\'') && (s.back() == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
    return s;
}

std::string makeString(Value value) {
    if (value.getType() == String) {
        return stripDoubleQuotes(value.toString());
    } else if (value.numeric()) {
        return value.coerceToString();
    } else if (value.getType() == jstNULL) {
        return "null";
    } else if (value.getType() == Undefined) {
        return "undefined";
    } else if (value.getType() == Array) {
        std::string res = "";
        bool first = true;
        for (Value v : value.getArray()) {
            if (!first) {
                res += ",";
            } else {
                first = false;
            }
            res += makeString(v);
        }
        return res;
    } else if (value.getType() == Bool) {
        return (value.getBool() ? "true" : "false");
    } else {
        return value.coerceToString();  // TODO
    }
}


Value makeNumeric(Value value) {
    if (value.numeric()) {
        return value;
    } else if (value.getType() == jstNULL) {
        return Value(0);
    } else if (value.getType() == Bool) {
        return value.getBool() ? Value(1) : Value(0);
    } else {
        // TODO should never get here
        throw std::runtime_error("Type error");
    }
}

bool isString(Value v) {
    if (v.getType() != String) {
        return false;
    }
    std::string s = makeString(v);
    std::cout << s << std::endl;
    return (((s != "NaN") && (s != "Infinity")) && (s != "-Infinity"));
}

bool countsAsNumber(Value v) {
    if (v.getType() == String) {
        return ((makeString(v) == "Infinity") || (makeString(v) == "-Infinity"));
    }
    return (v.numeric() || ((v.getType() == Bool) || (v.getType() == jstNULL)));
}


/*
 * The following are comparison helper functions. For now they are using value::compare. We need to
 * look for edge cases in Javascript that this might not cover.
 */

const Value BinaryOperator::evaluateGreaterThan(Scope* scope) const {
    const Value leftValue = this->getLeftChild()->evaluate(scope);
    const Value rightValue = this->getRightChild()->evaluate(scope);
    return Value(Value::compare(leftValue, rightValue) > 0);
}

const Value BinaryOperator::evaluateGreaterThanEquals(Scope* scope) const {
    const Value leftValue = this->getLeftChild()->evaluate(scope);
    const Value rightValue = this->getRightChild()->evaluate(scope);
    return Value(Value::compare(leftValue, rightValue) >= 0);
}

const Value BinaryOperator::evaluateLessThan(Scope* scope) const {
    const Value leftValue = this->getLeftChild()->evaluate(scope);
    const Value rightValue = this->getRightChild()->evaluate(scope);
    return Value(Value::compare(leftValue, rightValue) < 0);
}

const Value BinaryOperator::evaluateLessThanEquals(Scope* scope) const {
    const Value leftValue = this->getLeftChild()->evaluate(scope);
    const Value rightValue = this->getRightChild()->evaluate(scope);
    return Value(Value::compare(leftValue, rightValue) <= 0);
}

const Value BinaryOperator::evaluateObjectAccessor(Scope* scope) const {
    std::string objectPathString = BinaryOperator::generateNestedField(this, scope);
    int rootObjIndex = objectPathString.find_first_of('.');
    std::string fieldPathString = objectPathString.substr(rootObjIndex + 1),
                objectString = objectPathString.substr(0, rootObjIndex);
    FieldPath fieldPath = FieldPath(fieldPathString);
    Value object = scope->get(StringData(objectString));
    Document doc = object.getDocument();
    return doc.getNestedField(fieldPath);
}

std::string BinaryOperator::generateNestedField(const Node* head, Scope* scope) {
    std::string cur = (head->getName()).rawData();
    std::string leftNestedField, rightNestedField;
    if (cur == ".") {
        leftNestedField = BinaryOperator::generateNestedField(
            (checked_cast<const BinaryOperator*>(head))->getLeftChild(), scope);
        rightNestedField = BinaryOperator::generateNestedField(
            (checked_cast<const BinaryOperator*>(head))->getRightChild(), scope);
    } else if (cur == "[") {
        cur = ".";
        leftNestedField = BinaryOperator::generateNestedField(
            (checked_cast<const BinaryOperator*>(head))->getLeftChild(), scope);
        Value rightChildValue =
            (checked_cast<const BinaryOperator*>(head))->getRightChild()->evaluate(scope);
        rightNestedField = rightChildValue.coerceToString();
    } else {
        leftNestedField = "";
        rightNestedField = "";
    }
    return leftNestedField + cur + rightNestedField;
}

}  // namespace tinyjs
}  // namespace mongo
