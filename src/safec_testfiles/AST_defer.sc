#include <stdio.h>

void DEFER_FUNCTION_SCOPE(void)
{
    printf("function scope 1\n");
    defer printf("function scope 3\n");
    printf("function scope 2\n");
}

void DEFER_SIMPLE_SCOPE_ONLY(void)
{
    {
        printf("scope 1\n");
        defer printf("scope 3\n");
        printf("scope 2\n");
    }
}

void DEFER_SIMPLE_SCOPE(void)
{
    printf("function scope start\n");

    {
        printf("scope 1\n");
        defer printf("scope 3\n");
        printf("scope 2\n");
    }

    printf("function scope end\n");
}

// TODO: simple scope with declarations etc...
