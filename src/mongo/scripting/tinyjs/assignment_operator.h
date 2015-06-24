#include <string>
#include "mongo/db/pipeline/value.h"
#include "binary_operator.h"

namespace mongo {

class AssignmentOperator: public BinaryOperator {
public:
    enum AssignmentType {
        EQ
    };
    AssignmentOperator();
    Value *returnValue() const;
private:
    AssignmentType _assignmentType;
};

}
