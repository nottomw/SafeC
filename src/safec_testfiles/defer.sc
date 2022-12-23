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
    mutex_lock();
    defer mutex_unlock();

    if (arg == 0)
    {
        return;
    }

    printf("end of function...\n");
}

void test_function_2(int arg)
{
    int fp = file_open();
    defer file_close(fp);

    if (arg == 0)
    {
        return;
    }

    printf("end of function...\n");
}

void test_function_3(int a, int b)
{
    int i = 0;
    for (i = 0; i < 10; ++i)
    {
        defer printf("deferred loop print at i = %d\n", i);
        if (i == a)
        {
            continue;
        }

        if (i == b)
        {
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
        printf("in while loop\n");
        i = 1;
    }

    // TODO: this is not allowed - declarations only top of scope, allow
    // int j = 5;

    i = 0;
    do
    {
        defer printf("do..while loop end\n");
        printf("in do..while loop\n");
        i = 1;
    } while (i == 0);
}

void test_function_4(void)
{
    int a = 5;
    (void)a;
    {
        int *a = malloc(sizeof(int));
        defer free(a);
        printf("malloc-free test: a\n");
        {
            int *b = malloc(sizeof(int));
            defer free(b);
            printf("malloc-free test: b\n");
        }
    }
}

int test_function_5(const int arg)
{
    defer printf("multiple returns - depending on arg: %d\n", arg);

    if (arg == 0)
    {
        return 0;
    }
    else if (arg < 5)
    {
        return 5;
    }

    return 10;
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

    test_function_5(0);
    test_function_5(2);
    test_function_5(7);

    return 0;
}
