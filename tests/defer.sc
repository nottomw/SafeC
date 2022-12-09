#include <stdio.h>

void mutex_lock()
{
    printf("mutex lock\n");
}

void mutex_unlock()
{
    printf("mutex unlock\n");
}

void test_function(bool arg)
{
    mutex_lock();
    defer mutex_lock();

    if (arg == false)
    {
        return;
    }

    printf("end of function...\n");
}

int main(void)
{
    {
        mutex_lock();
        defer mutex_unlock();

        printf("this is guarded by mutex...\n");
    }

    printf("this is not guarded by mutex...\n");

    test_function(true);
    test_function(false);

    return 0;
}
