#include <stdio.h>

int globalDeclarationNoAssignment;
int globalDeclaration = 5;

void FUNCTION_NO_RETURN_NO_PARAMS(void)
{
    // nothing
}

void** FUNCTION_WITH_VOIDPTR_RETURN_NO_PARAMS(void)
{
    // nothing
}

int FUNCTION_WITH_DECLARATIONS(const int paramCondition, float *foo, double **bar)
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
        // i = i++; // TODO
    }

    // if (paramCondition == 42) // kConstant - fails now
    if (paramCondition == SOME_TEST_VAL)
    {
        int resFromCalledFunWithExprInFun = calledFunction(paramCondition + 666); // expression in function params
        int resFromCalledFun = calledFunction(paramCondition, foo + bar);
        int resFromCalledFun = calledFunction(paramCondition, foo);
        int resFromAnother = someFunctionWithNoParams();
        resFromCalledFun = calledFunctionSecond(paramCondition);
        if (resFromCalledFun != 0)
        {
            return resFromCalledFun;
        }
    }

    return 124;
}
