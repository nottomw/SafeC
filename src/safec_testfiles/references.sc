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
    {
        struct SomeStruct &secondRef = s;
        printf("address of s: %p, address of secondRef: %p\n", &s, &secondRef);
    }

    return s.a;
}

int main(void)
{
    struct SomeStruct *s = malloc(sizeof(struct SomeStruct));
    struct SomeStruct &sRef = *s;
    int x = fun2(*s);

    assert(s != NULL);

    sRef.a = 5;
    sRef.b = 'A';

    printf("x = %d\n", x);

    fun(x);
    printf("x = %d\n", x);

    // TODO: reference assignment should fail

    return 0;
}
