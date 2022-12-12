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
