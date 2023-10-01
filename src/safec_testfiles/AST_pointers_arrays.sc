void TEST_LOOP_ITER_POINTERS(void)
{
    // int a, b, c; // TODO: not handled
    int a = 3;
    int b = 6;
    int c = 9;
    int *d = &c;
    int **e = &d;
    int arrWithConsts[] = { 1, 2, 3, 4 };
    int *arr[] = { &a, &b, &c, *d, **e, (d + 128), NULL };

    int i = 0;
    for (i = 0; /* no cond */; i++)
    {
        int arrVal = *arr[i]; // init expr - might be null, just testing

        if (arr[i] == NULL)
        {
            break;
        }

        arrVal = *arr[i]; // assign expr
        printf("i = %d, arrVal = %d\n", i, arrVal);
    }
}
