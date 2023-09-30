#include <stdio.h>
#include <stdlib.h>

void mutex_lock()
{
    printf("mutex lock\n");
}

void mutex_unlock()
{
    printf("mutex unlock\n");
}

int file_open()
{
    static int fd = 0;
    int fd_ret = fd;

    printf("file open: %d\n", fd);
    fd++;

    return fd_ret;
}

void file_close(int fd)
{
    printf("file close: %d\n", fd);
}

void test_function_1(int arg)
{
    printf("------> TEST 1: test function with simple defer\n");

    mutex_lock();
    defer mutex_unlock();

    if (arg == 0)
    {
        printf("TEST 1: mutex unlock after this print\n");
        return;
    }

    printf("TEST 1: mutex unlock after this print\n");
}

void test_function_2(int arg)
{
    int fp = 0;

    printf("------> TEST 2: test function with simple defer, arguments in defer call\n");

    fp = file_open();
    defer file_close(fp);

    if (arg == 0)
    {
        printf("TEST 2: file close after this print\n");
        return;
    }

    printf("TEST 2: file close after this print\n");
}

void test_function_3(int a, int b)
{
    int i = 0;

    printf("------> TEST 3: test function defer in loops\n");

    for (i = 0; i < 10; ++i)
    {
        defer printf("deferred loop print at i = %d\n", i);
        if (i == a)
        {
            printf("TEST 3: for loop: print after this print\n");
            continue;
        }

        if (i == b)
        {
            printf("TEST 3: for loop: print after this print\n");
            break;
        }
    }

    // invalid construct - maybe should fail on safec parsing step
    //    for (i = 0; i < 10; ++i)
    //        defer printf("%d\n", i);

    i = 0;
    while (i == 0)
    {
        defer printf("while loop end\n");
        i = 1;
        printf("TEST 3: while loop: print after this print\n");
    }

    // TODO: this is not allowed - declarations only top of scope, allow (C99?)
    // int j = 5;

    i = 0;
    do
    {
        defer printf("do..while loop end\n");
        printf("in do..while loop\n");
        i = 1;
        printf("TEST 3: do..while loop: print after this print\n");
    } while (i == 0);
}

void test_function_4(void)
{
    int a = 5;
    (void)a;

    printf("------> TEST 4: defer with malloc/free\n");

    {
        int *a = malloc(sizeof(int));
        defer free(a);

        {
            int *b = malloc(sizeof(int));
            defer free(b);
            printf("TEST 4: free b after this print\n");
        }

        printf("TEST 4: free a after this print\n");
    }
}

int test_function_5(const int arg)
{
    printf("------> TEST 5: defer with multiple returns (arg dependent)\n");

    defer printf("multiple returns - depending on arg: %d\n", arg);

    if (arg == 0)
    {
        printf("TEST 5: print after this print (arg == 0)\n");
        return 0;
    }
    else if (arg < 5)
    {
        printf("TEST 5: print after this print (arg < 5)\n");
        return 5;
    }

    printf("TEST 5: print after this print (arg >= 5)\n");
    return 10;
}

void test_function_6(void)
{
    printf("------> TEST 6: double defer, should appear in reverse order\n");
    {
        defer printf("deferred call odd 1\n");
        defer printf("deferred call odd 2\n");
        defer printf("deferred call odd 3\n");
    }

    {
        defer printf("deferred call even 1\n");
        defer printf("deferred call even 2\n");
        defer printf("deferred call even 3\n");
        defer printf("deferred call even 4\n");
    }
}

int main(void)
{
    test_function_1(0);
    test_function_1(1);

    test_function_2(0);
    test_function_2(1);

    test_function_3(3, 6);
    test_function_3(6, 3);

    test_function_4();

    (void)test_function_5(0);
    (void)test_function_5(2);
    (void)test_function_5(7);

    test_function_6();

    return 0;
}

// TODO: fails with new AST parser
