#include <stdio.h>

int globalDeclaration = 5;

int FUNCTION_WITH_DECLARATIONS(const int paramCondition)
{
    int varWithNoValue;
    int varWithAssignmentFromOtherIdentifier = varWithNoValue;
    int i = 0;
    int *ptrAssignment = &varWithNoValue;
    int **ptrAssignmentSecond = &ptrAssignment;

    // const int valueDeclarationWithOperandsOnRhs = (i + 123); // TODO - rhs expression

    varWithNoValue = 123; // assignment
    varWithNoValue += 123; // unary op assignment

    for (i = 0; i < a; i++)
    {
        printer("num: %d\n", i);
    }

    if (paramCondition == 42)
    {
        int resFromCalledFun = calledFunction(paramCondition + 666);
        if (resFromCalledFun != 0)
        {
            return resFromCalledFun;
        }
    }

    return 124;
}
