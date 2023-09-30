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
    switch(param)
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
    switch(param)
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

void SWITCH_CASE_FALLTHROUGH_NO_BREAKS(int param)
{
    switch(param)
    {
        case 1:
        case 2:
        case 3:
        default:
            ;
    }
}

void SWITCH_CASE_FALLTHROUGH_NO_BREAKS_NO_DEFAULT(int param)
{
    switch(param)
    {
        case 1:
        case 2:
        case 3:
            ; // without empty statement syntax error
    }
}

void SWITCH_CASE_WITH_SCOPES_AND_FALLTHROUGH(int param)
{
    switch(param)
    {
        case 1:
            {
                printf("1\n");
                break;
            }

        case 2:
        case 3:
        case 4:
            {
                printf("2 3 4 fall\n");
                printf("second call\n");
            }
            break;
    }
}

void SWITCH_CASE_FALLTHROUGH_TO_DEFAULT(int param)
{
    switch(param)
    {
        case 1:
        case 2:
        default:
        {
            printf("1 or default\n");
        }
        break;
    }
}
