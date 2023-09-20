#include <stdio.h>

void TEST_LOOP_FOR(void)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        printf("i = %d\n", i);
    }
}

void TEST_LOOP_FOR_BREAK(void)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        printf("i = %d\n", i);
        if (i == 5)
        {
            break;
        }
    }
}

void TEST_LOOP_FOR_RETURN(void)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        printf("i = %d\n", i);
        if (i == 5)
        {
            return;
        }
    }
}

void TEST_LOOP_FOR_CONTINUE(void)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        if (i == 5)
        {
            continue;
        }

        printf("i = %d\n", i);
    }
}

void TEST_LOOP_WHILE(void)
{
    int i = 0;
    while(i < 5)
    {
        i += 1;
    }
}

void TEST_LOOP_WHILE_BREAK(void)
{
    int i = 0;
    while(i < 5)
    {
        i += 1;
        if (i == 2)
        {
            break;
        }
    }
}

void TEST_LOOP_WHILE_RETURN(void)
{
    int i = 0;
    while(i < 5)
    {
        i += 1;
        if (i == 3)
        {
            return;
        }
    }
}

void TEST_LOOP_WHILE_CONTINUE(void)
{
    int i = 0;
    while(i < 5)
    {
        i += 1;
        if (i == 4)
        {
            continue;
        }
        printf("i = %d\n", i);
    }
}
