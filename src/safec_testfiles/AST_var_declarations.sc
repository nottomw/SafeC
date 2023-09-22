#include <stdio.h>

int globalDeclarationNoAssignment;
int globalDeclaration = 5;

void FUNCTION_NO_RETURN_NO_PARAMS(void)
{
    // nothing
}

void **FUNCTION_WITH_VOIDPTR_RETURN_NO_PARAMS(void)
{
    // nothing
}

int FUNCTION_RETURNING_CONSTANT(void)
{
    return 123;
}

int FUNCTION_RETURNING_CONSTANT_EXPRESSION(void)
{
    return 123 + 456;
}

int FUNCTION_RETURNING_DEREF(void)
{
    int *ptr = &globalDeclaration;
    return *ptr;
}

int FUNCTION_RETURNING_DOUBLE_DEREF(void)
{
    int *ptr = &globalDeclaration;
    int *ptrptr = &ptr;
    return **ptrptr;
}

int FUNCTION_RETURNING_PREINCREMENT(void)
{
    return ++globalDeclaration;
}

int FUNCTION_RETURNING_POSTINCREMENT(void)
{
    return globalDeclaration++;
}

int FUNCTION_WITH_DECLARATIONS(const int paramCondition, float *foo, double **bar)
{
    int varWithNoValue;
    int varWithAssignmentFromOtherIdentifier = varWithNoValue;
    int i = 0;
    int *ptrAssignment = &varWithNoValue;
    int **ptrAssignmentSecond = &ptrAssignment;
    int doubleDerefUnaryOp = **ptrAssignmentSecond;

    const int valueDeclarationWithOperandsOnRhs = (i + 123);
    int valueDeclWithBinaryOpsRhs = i + 555 + 666;

    varWithNoValue = 123;  // assignment
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
    }

    for (i = SOME_INIT_VAL; i < a; i++)
    {
        printer("num: %d\n", i); // plain call
        i = i++;
    }

    if (paramCondition == 42)
    {
        printer("if with constant\n");
    }

    if (paramCondition == SOME_TEST_VAL)
    {
        int resFromCalledFunWithExprInFun = calledFunction(paramCondition + 666);
        int resFromCalledFunWithSumInArg = calledFunction(paramCondition, foo + bar);
        int resFromCalledFun = calledFunction(paramCondition, foo);
        int resFromAnother = someFunctionWithNoParams();
        resFromCalledFun = calledFunctionSecond(paramCondition);

        resFromCalledFunWithExprInFun = calledFunction(paramCondition + 666 + 777);

        if (resFromCalledFun != 0)
        {
            return FUNCTION_RETURNING_PREINCREMENT();
        }
    }

    ++i;

    return 124;
}

int gSomeGlobal;
int gSomeGlobalInitialized = 123;

void FUNCTION_WITH_EMPTY_STATEMENTS_AND_GLOBAL_SET(const int someArg)
{
    ; // empty
    gSomeGlobal = someArg;
    gSomeGlobalInitialized = someArg + 500;
    ; // empty

    if (someArg == 999)
    {
        ; // empty
    }

    ; // empty
}

void FUNCTION_WITH_MULTIPLE_SAME_TYPE_VARS(void)
{
    // int a, b, c; // TODO: handle
}
