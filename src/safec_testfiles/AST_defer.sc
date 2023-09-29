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

void DEFER_CONDITION(int someParam)
{
    printf("fun start\n");

    if (someParam == 5)
    {
        printf("1\n");
        defer printf("3\n");
        printf("2\n");
    }

    printf("fun end\n");
}

void DEFER_CONDITION_MULTIPLE(int someParam)
{
    defer printf("last");

    if (someParam > 10)
    {
        defer printf("2\n");
        if (someParam < 5)
        {
            defer printf("2\n");
            printf("1\n");
        }
    }
}

void DEFER_LOOP(void)
{
    int i = 0;
    for(i = 0; i < 5; i++)
    {
        defer printf("2\n");
        printf("1\n");
    }

    i = 0;
    while(i < 5)
    {
        defer printf("2\n");
        printf("1\n");
        i++;
    }
}

void DEFER_LOOP_BREAK(int param)
{
    int i = 0;
    for(i = 0; i < 5; i++)
    {
        defer printf("2\n");

        if (i == param)
        {
            break;
        }

        printf("1\n");
    }

    i = 0;
    while(i < 5)
    {
        defer printf("2\n");

        if (i == param)
        {
            break;
        }

        printf("1\n");
        i++;
    }
}

void DEFER_LOOP_CONTINUE(int param)
{
    int i = 0;
    for(i = 0; i < 5; i++)
    {
        defer printf("2\n");

        if (i == param)
        {
            continue;
        }

        printf("1\n");
    }

    i = 0;
    while(i < 5)
    {
        defer printf("2\n");

        if (i == param)
        {
            continue;
        }

        printf("1\n");
        i++;
    }

}

void DEFER_MULTIPLE(void)
{
    defer printf("3\n");
    defer printf("2\n");
    defer printf("1\n");
}

// TODO: handle switch..case in parser
//void DEFER_SWITCH_CASE(int param)
//{
//    defer printf("last\n");

//    switch(param)
//    {
//        case 0:
//            printf("0\n");
//        break;

//        case 1:
//            printf("0\n");
//        break;

//        default:
//            printf("default\n");
//            break;
//    }
//}

void DEFER_STRANGE(void)
{
    int i = 0;

    // no-scope loops and conds

    for(i = 0; i < 5; i++)
        defer printf("%d\n", i);

    if (i == 0)
        defer printf("deferred print in cond\n");

    defer printf("last statement in function\n");
}

void DEFER_WITH_COMPL_STATEMENTS(void)
{
    int counter = 0;
    int counterSecond = 5;

    printf("fun start\n");

    {
        defer counter += 1;
        printf("counter: %d\n", counter);
    }
    printf("counter: %d\n", counter);

    {
        defer counter = counterSecond + counter;
        printf("counter: %d\n", counter);
    }
    printf("counter: %d\n", counter);
}

// TODO: simple scope with declarations etc...
