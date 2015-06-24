#include <string>
#include "mongo/db/pipeline/value.h"
#include "binary_operator.h"

namespace mongo {

class ArithmeticOperator: public BinaryOperator {
public:
    enum ArithmeticType {
        ADDITIVE,
        MULTIPLICATIVE,
    };
    ArithmeticOperator();
    Value *returnValue() const;
private:
    ArithmeticType _arithmeticType;
};

}
