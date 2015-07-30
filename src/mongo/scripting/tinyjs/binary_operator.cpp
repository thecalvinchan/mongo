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

#include <boost/lexical_cast.hpp>
#include <math.h>

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

bool isZero(Value value) {
    return (((value.getType() == NumberInt) && (value.getInt() == 0)) ||
            ((value.getType() == NumberDouble) && (value.getDouble() == 0)) ||
            ((value.getType() == NumberLong) && (value.getLong() == 0)) ||
            (value.getType() == jstNULL) ||
            ((value.getType() == Bool) && value.getBool() == false));
}

bool isNegative(Value value) {
    // uses coerceToDouble so that infinity comparisons work
    return ((value.numeric() && (value.coerceToDouble() < 0.0)));
}

std::string makeString(Value value) {
    if (value.getType() == String) {
        std::string s = value.toString();
        if (((s.front() == '"') && (s.back() == '"')) ||
            ((s.front() == '\'') && (s.back() == '\''))) {
            s = s.substr(1, s.size() - 2);
        }
        return s;
    } else if ((value.getType() == NumberDouble) && (isnan(value.getDouble()))) {
        return "NaN";
    } else if ((value.getType() == NumberDouble) && (value.getDouble() == std::numeric_limits<double>::infinity())) {
        return "Infinity";
    } else if ((value.getType() == NumberDouble) && (value.getDouble() == -std::numeric_limits<double>::infinity())) {
        return "-Infinity";
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
    return (((s != "NaN") && (s != "Infinity")) && (s != "-Infinity"));
}

bool countsAsNumber(Value v) {
    if (v.getType() == String) {
        return ((makeString(v) == "Infinity") || (makeString(v) == "-Infinity"));
    }
    return (v.numeric() || ((v.getType() == Bool) || (v.getType() == jstNULL)));
}

bool isFalse(Value v) {
    return (isZero(v) || (v.getType() == Undefined) ||
            (v.getType() == NumberDouble && isnan(v.getDouble())));
}

bool strictlyEqual(Value leftValue, Value rightValue) {
    if (((leftValue.getType() == NumberDouble && leftValue.getDouble() == std::nan(""))) ||
        ((rightValue.getType() == NumberDouble && rightValue.getDouble() == std::nan("")))) {
        return false;
    } else if ((leftValue.getType() == Array) || (rightValue.getType() == Array)) {
        return false;
    } else if (leftValue.getType() != rightValue.getType()) {
        return false;
    } else {
        return Value::compare(leftValue, rightValue) == 0;
    }
}

bool looselyEqualNumberString(Value numberValue, std::string s) {
    switch (numberValue.getType()) {
        case NumberInt: {
            if (s == "") {
                return (numberValue.getInt() == 0);
            }

            int rightInt;
            try {
                rightInt = boost::lexical_cast<int>(s);
                return numberValue.getInt() == rightInt;
            } catch (boost::bad_lexical_cast& e) {
                return false;
            }
        }
        case NumberDouble: {
            if (s == "") {
                return (numberValue.getDouble() == 0);
            }

            double rightDouble;
            try {
                rightDouble = boost::lexical_cast<double>(s);
                return numberValue.getDouble() == rightDouble;
            } catch (boost::bad_lexical_cast& e) {
                return false;
            }
        }
        case NumberLong: {
            if (s == "") {
                return (numberValue.getLong() == 0);
            }

            double rightLong;
            try {
                rightLong = boost::lexical_cast<long>(s);
                return numberValue.getLong() == rightLong;
            } catch (boost::bad_lexical_cast& e) {
                return false;
            }
        }
        default:
            verify(false);
    }
}

bool looselyEqualNumberBool(Value numberValue, bool b) {
    int boolNumber = (b ? 1 : 0);
    // TODO behavior for 1.0 and 1.5 ?
    return (numberValue.coerceToInt() == boolNumber);
}

bool looselyEqualStringBool(std::string s, bool b) {
    int boolInt = (b ? 1 : 0);
    if ((s == "") && (!b)) {
        return true;
    }
    try {
        int stringInt = boost::lexical_cast<int>(s);
        return stringInt == boolInt;
    } catch (boost::bad_lexical_cast& e) {
        return false;
    }
    // TODO behavior for 1.0 and 1.5 ?
}


bool looselyEqual(Value leftValue, Value rightValue) {
    if (leftValue.getType() == Array) {
        if (rightValue.getType() != Array) {
            if (leftValue.getArray().size() == 0) {
                leftValue = Value(false);
            } else if (leftValue.getArray().size() == 1) {
                leftValue = leftValue.getArray()[0];
                if ((leftValue.getType() == Array) && leftValue.getArray().size() == 0) {
                    leftValue = Value(false);
                }
            } else {
                return false;
            }

        } else {
            // 2 array literals can't be equal
            return false;
        }
    } else if (rightValue.getType() == Array) {
        if (rightValue.getArray().size() == 0) {
            rightValue = Value(false);
        } else if (rightValue.getArray().size() == 1) {
            rightValue = rightValue.getArray()[0];
            if ((rightValue.getType() == Array) && rightValue.getArray().size() == 0) {
                rightValue = Value(false);
            }
        } else {
            return false;
        }
    }

    if (leftValue.getType() == Undefined) {
        if (rightValue.getType() == jstNULL) {
            return true;
        }
    }

    if (leftValue.getType() == jstNULL) {
        if (rightValue.getType() == Undefined) {
            return true;
        }
    }

    if (leftValue.numeric()) {
        if (rightValue.getType() == String) {
            return looselyEqualNumberString(leftValue, rightValue.getString());
        } else if (rightValue.getType() == Bool) {
            return looselyEqualNumberBool(leftValue, rightValue.getBool());
        }
    }

    if (leftValue.getType() == String) {
        if (rightValue.numeric()) {
            return looselyEqualNumberString(rightValue, leftValue.getString());
        } else if (rightValue.getType() == Bool) {
            return looselyEqualStringBool(leftValue.getString(), rightValue.getBool());
        }        
    }

    if (leftValue.getType() == Bool) {
        if (rightValue.numeric()) {
            return looselyEqualNumberBool(rightValue, leftValue.getBool());
        } else if (rightValue.getType() == String) {
            return looselyEqualStringBool(rightValue.getString(), leftValue.getBool());
        }
    }

    return Value::compare(leftValue, rightValue) == 0;
}

}  // namespace tinyjs
}  // namespace mongo
