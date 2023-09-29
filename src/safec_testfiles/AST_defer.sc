#include <stdio.h>

void DEFER_SIMPLE_SCOPE(void)
{
    printf("one\n");
    defer printf("three\n");
    printf("two\n");
}
