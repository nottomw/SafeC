// switch-case test cases

#include <stdio.h>

void SWITCH_CASE_SIMPLE(int param)
{
    switch(param)
    {
        case 0:
            printf("0\n");
            break;

        case 1:
            printf("1\n");
            break;

        default:
            printf("default\n");
            break;
    }
}

void SWITCH_CASE_IDENTIFIER_LABEL(int param)
{
    #define ZERO 0

    switch(param)
    {
        case ZERO:
            printf("0\n");
            break;

        default:
            printf("default\n");
            break;
    }
}

void SWITCH_CASE_IDENTIFIER_EXPR(int param)
{
    switch(param)
    {
        case 3 + 4:
            printf("3 + 4\n");
            break;

        default:
            printf("default\n");
            break;
    }

    printf("end\n");
}

void SWITCH_CASE_BINARY(int param)
{
    switch(param + 666)
    {
        case 667:
            printf("667\n");
            break;

        default:
            printf("default\n");
            break;
    }

    printf("end\n");
}

void SWITCH_CASE_FALLTHROUGH_EMPTY(int param)
{
    switch(param + 1 + 2)
    {
        case 1:
            printf("1\n");
        break;

        case 2:
        case 3:
        case 4:
            printf("2 or 3 or 4\n");
        break;
    }
}

void SWITCH_CASE_FALLTHROUGH_BODY(int param)
{
    switch(param + 1 + 2)
    {
        case 1:
            printf("1\n");
        break;

        case 2:
            printf("2 fall ");
        case 3:
            printf("3 fall ");
        case 4:
        case 5:
            printf("4 5\n");
        break;
    }
}
