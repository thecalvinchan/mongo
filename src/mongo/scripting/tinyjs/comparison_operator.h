#include <string>
#include "mongo/db/pipeline/value.h"
#include "binary_operator.h"

namespace mongo {

class ComparisonOperator: public BinaryOperator {
public:
    enum ComparisonType {
        EQ_LOOSE,
        EQ_STRICT,
        LTE,
        LT,
        GT,
        GTE,
        NOT_EQ_LOOSE,
        NOT_EQ_STRICT
    };
    ComparisonOperator();
    Value *returnValue() const;
private:
    ComparisonType _comparisonType;
};

}
