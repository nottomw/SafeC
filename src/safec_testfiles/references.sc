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

void add(int &result, const int &a, const int &b)
{
    result = a + b;
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

    {
        int sum = 0;
        // add(sum, 5, 10); // should be allowed- create tmps for 5 & 10
        const int addOne = 5;
        int addTwo = 5;

        // should change to `add(&sum, &addOne, &addTwo)`
        add(sum, addOne, addTwo);

        printf("adding %d + %d = %d\n", addOne, addTwo, sum);
    }

    return 0;
}
