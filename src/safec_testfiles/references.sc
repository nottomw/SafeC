#include <stdio.h>
#include <assert.h>

void fun(int &intReference)
{
    intReference = 125;
}

struct SomeStruct{
    int a;
    char b;
};

int fun2(struct SomeStruct &structReference)
{
    {
        struct SomeStruct &secondRef = structReference;
        printf("address of structReference: %p, address of secondRef: %p\n",
            &structReference,
            &secondRef);
    }

    return structReference.a;
}

int main(void)
{
    struct SomeStruct *structPtr = malloc(sizeof(struct SomeStruct));
    struct SomeStruct &sRef = *structPtr;
    int x = 0;

    assert(structPtr != NULL);

    // calling reference function parameter
    x = fun2(*structPtr);

    sRef.a = 5;
    sRef.b = 'A';

    printf("x = %d\n", x);

    fun(x);
    printf("x = %d\n", x);

    // TODO: reference assignment should fail

    return 0;
}
