#include <stdio.h>
#include <assert.h>

void fun(int &x)
{
    x = 125;
}

struct SomeStruct{
    int a;
    char b;
};

int fun2(struct SomeStruct &s)
{
    return s.a;
}

int main(void)
{
    struct SomeStruct *s = malloc(sizeof(struct SomeStruct));
    assert(s != NULL);

    struct SomeStruct &sRef = *s;
    sRef.a = 5;
    sRef.b = 'A';

    int x = fun2(*s);
    printf("x = %d\n", x);

    fun(x);
    printf("x = %d\n", x);

    // TODO: reference assignment should fail

    return 0;
}
