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
        int varInsideLoop = i;
        varInsideLoop = i; // reassignment
        printer("num: %d\n", i);
    }

    for (; i < a; i++) // empty statement
    {
        i = 2;
        // nothing
    }

    for (i = SOME_INIT_VAL; i < a; i++)
    {
        printer("num: %d\n", i); // plain call
    }

    // if (paramCondition == 42) // kConstant - fails now
    if (paramCondition == SOME_TEST_VAL)
    {
        // int resFromCalledFunWithExprInFun = calledFunction(paramCondition + 666); // expression in function params
        int resFromCalledFun = calledFunction(paramCondition); // TODO
        resFromCalledFun = calledFunctionSecond(paramCondition);
        if (resFromCalledFun != 0)
        {
            return resFromCalledFun;
        }
    }

    return 124;
}
