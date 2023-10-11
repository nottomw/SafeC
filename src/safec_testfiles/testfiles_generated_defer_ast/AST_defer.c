#include <stdio.h>

void DEFER_FUNCTION_SCOPE(void)
{
    printf("function scope 1\n");
    printf("function scope 2\n");
 printf("function scope 3\n");
}

void DEFER_SIMPLE_SCOPE_ONLY(void)
{
    {
        printf("scope 1\n");
        printf("scope 2\n");
 printf("scope 3\n");
    }
}

void DEFER_SIMPLE_SCOPE(void)
{
    printf("function scope start\n");

    {
        printf("scope 1\n");
        printf("scope 2\n");
 printf("scope 3\n");
    }

    printf("function scope end\n");
}

void DEFER_CONDITION(int someParam)
{
    printf("fun start\n");

    if (someParam == 5)
    {
        printf("1\n");
        printf("2\n");
 printf("3\n");
    }

    printf("fun end\n");
}

void DEFER_CONDITION_MULTIPLE_NESTED(int someParam)
{

    if (someParam > 10)
    {
        if (someParam < 5)
        {
            printf("1\n");
 printf("2\n");
        }
 printf("3\n");
    }
 printf("last");
}

void DEFER_CONDITION_MULTIPLE(int someParam)
{

    if (someParam > 10)
    {
        printf("1.1");
 printf("1.2\n");
    }

    if (someParam < 5)
    {
        printf("2.1\n");
 printf("2.2\n");
    }
 printf("last");
}

void DEFER_LOOP(void)
{
    int i = 0;
    for (i = 0; i < 5; i++)
    {
        printf("1\n");
 printf("2\n");
    }

    i = 0;
    while (i < 5)
    {
        printf("1\n");
        i++;
 printf("2\n");
    }
}

void DEFER_LOOP_BREAK(int param)
{
    int i = 0;
    for (i = 0; i < 5; i++)
    {

        if (i == param)
        {
            break;
        }

        printf("1\n");
 printf("2\n");
    }

    i = 0;
    while (i < 5)
    {

        if (i == param)
        {
            break;
        }

        printf("1\n");
        i++;
 printf("2\n");
    }
}

void DEFER_LOOP_CONTINUE(int param)
{
    int i = 0;
    for (i = 0; i < 5; i++)
    {

        if (i == param)
        {
            continue;
        }

        printf("1\n");
 printf("2\n");
    }

    i = 0;
    while (i < 5)
    {

        if (i == param)
        {
            continue;
        }

        printf("1\n");
        i++;
 printf("2\n");
    }
}

void DEFER_MULTIPLE(void)
{
 printf("1\n");
 printf("2\n");
 printf("3\n");;
}

void DEFER_SWITCH_CASE(int param)
{

    switch (param)
    {
        case 0:
            printf("0\n");
            break;

        case 1:
            printf("0\n");
            break;

        default:
            printf("default\n");
            break;
    }
 printf("last\n");
}

void DEFER_STRANGE(void)
{
    int i = 0;

    // no-scope loops and conds

    for (i = 0; i < 5; i++)
 printf("%d\n", i);

    if (i == 0)
 printf("deferred print in cond\n");
 printf("last statement in function\n");;
}

void DEFER_WITH_COMPL_STATEMENTS(void)
{
    int counter = 0;
    int counterSecond = 5;

    printf("fun start\n");

    {
        printf("counter: %d\n", counter);
 counter += 1;;
    }
    printf("counter: %d\n", counter);

    {
        printf("counter: %d\n", counter);
 counter = counterSecond + counter;;
    }
    printf("counter: %d\n", counter);
}