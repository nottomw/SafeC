#include <stdio.h>
#include <assert.h>

// should change to `int *const intReference`
void fun(int &intReference)
{
    // should change to `*intReference = ...;`
    intReference = 125;
}

struct SomeStruct{
    int a;
    char b;
};

int fun2(struct SomeStruct &structReference)
{
    {
        // should change to `*secondRef = structReference;`
        struct SomeStruct &secondRef = structReference;
        printf("address of structReference: %p, address of secondRef: %p\n",
            &structReference, // should change to `structReference`
            &secondRef);
    }

    // should change to `structReference->a`
    return structReference.a;
}

int main(void)
{
    struct SomeStruct *structPtr = malloc(sizeof(struct SomeStruct));

    // should change to `*const sRef = structPtr`
    struct SomeStruct &sRef = *structPtr;
    int x = 0;

    assert(structPtr != NULL);

    // calling reference function parameter
    // should change to `fun2(structPtr)`
    x = fun2(*structPtr);

    // should change to `sRef->a`
    sRef.a = 5;
    sRef.b = 'A';

    printf("x = %d\n", x);

    // should change to `&x`
    fun(x);
    printf("x = %d\n", x);

    // TODO: reference assignment should fail
    // i.e. int &x = a; x = b;

    return 0;
}
