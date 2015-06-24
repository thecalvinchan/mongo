#include <string>
#include "mongo/db/pipeline/value.h"
#include "binary_operator.h"

namespace mongo {

class LogicalOperator: public BinaryOperator {
public:
    enum LogicalType {
        AND,
        OR
    };
    LogicalOperator();
    Value *returnValue() const;
private:
    LogicalType _logicalType;
};

}
