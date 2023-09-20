#include <stdio.h>

struct somestruct_t { int a; };

struct somestruct_t FUNCTION_WITH_STRUCT_PARAM(const struct somestruct_t structArg)
{
	struct somestruct_t t = arg;
	return t;
}

static void staticFun(void)
{
}

volatile int *const constPointerRetFun(void)
{
	return NULL;
}

void FUNCTION_WITH_NO_RETURN(void)
{
}

void *FUNCTION_WITH_VOIDPTR_RETURN(void)
{
	return NULL;
}

void **FUNCTION_WITH_POINTER_TO_POINTER(void)
{
	return NULL;
}

void FUNCTION_WITH_NO_RETURN_VOID_PTR_ARG(void *voidPtrArg)
{
}


void ***FUNCTION_WITH_VOIDPTR3_RETURN_VOID_PTR_ARG(void ***voidPtr3Arg, void *voidPtr1Arg)
{
    return voidPtr3Arg;
}

void FUNCTION_WITH_POINTER_PARAMS(int *intPtr, double **doublePtr, char ***charPtr)
{
}

int FUNCTION_WITH_LOOP(int a)
{
    int i = 0;
    for (i = 0; i < a; i++)
    {
        printer("num: %d\n", i);
    }
}

int main(int argc, char** argv)
{
    testfun(argc);

    return 0;
}
