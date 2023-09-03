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

    // varWithNoValue = 123; // assignment
    // varWithNoValue += 123; // unary op assignment

    //for (i = 0; i < a; i++) // kConstant - fails now
    //{
    //    printer("num: %d\n", i);
    //}

    for (i = SOME_INIT_VAL; i < a; i++)
    {
        printer("num: %d\n", i);
    }

    // if (paramCondition == 42) // kConstant - fails now
    if (paramCondition == SOME_TEST_VAL)
    {
        // int resFromCalledFunWithExprInFun = calledFunction(paramCondition + 666); // kConstant - fails now
        int resFromCalledFun = calledFunction(paramCondition);
        if (resFromCalledFun != 0)
        {
            return resFromCalledFun;
        }
    }

    return 124;
}
