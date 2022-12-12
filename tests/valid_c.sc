// single line comment

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
    mutex_unlock();

    if (arg == false)
    {
        return;
    }

    printf("end of function...\n");
}

void test_function_file(int arg)
{
    int fp = file_open();
    file_close(fp);

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
        mutex_unlock();

        printf("this is guarded by mutex...\n");
    }

    printf("this is not guarded by mutex...\n");

    test_function(true);
    test_function(false);

    return 0;
}
