#include <stdio.h>

void mutex_lock()
{
    printf("mutex lock\n");
}

void mutex_unlock()
{
    printf("mutex unlock\n");
}

void test_function(int arg)
{
    mutex_lock();
    defer mutex_unlock();

    if (arg == 0)
    {
        return;
    }

    printf("end of function...\n");
}

void test_function_file(int arg)
{
    int fp = file_open();
    defer file_close(fp);

    if (arg == 0)
    {
        return;
    }

    printf("end of function...\n");
}

void test_function_break_continue(int a, int b)
{
    int i = 0;
    for(i = 0; i < 10; ++i)
    {
        defer printf("deferred loop print at i = %d\n", i);
        if (i == 5)
        {
            continue;
        }

        if (i == 8)
        {
            break;
        }
    }

    for(i = 0; i < 10; ++i)
        defer printf("%d\n", i);
    
    i = 0;
    while(i == 0)
    {
        defer printf("while loop end\n");
        printf("in while loop\n");
        i = 1;
    }

    i = 0;
    do
    {
        defer printf("do..while loop end\n");
        printf("in do..while loop\n");
        i = 1;
    } while(i == 0);
}

void test_function_nested_defer(void)
{
    int a = 5;
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

int test_defer_with_rv(const int arg)
{
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
    {
        mutex_lock();
        defer mutex_unlock();

        printf("this is guarded by mutex...\n");
    }

    printf("this is not guarded by mutex...\n");

    test_function(1);
    test_function(0);

    return 0;
}
