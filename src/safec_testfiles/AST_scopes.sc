// test statements placed in correct scopes

void FUNCTION_SCOPES(void)
{
    int a = 5;
    int b = 6;

    a = a + b;

    {
        int c = 123;
        c += 1;
    }

    b = 666;

    {
        int d = 654;
        {
            ++d;
        }
        d = d + b;
    }

    b = 777;
}
