#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int i = 0;
    int odd = 0;
    int doinb = 0;
    scanf("%d", &i);
    for (; i > 0; i--)
    {
        if (i % 2 == 0)
        {
            doinb++;
        }
        else
        {
            odd++;
        }
    }
    printf("%d %d", odd, doinb);
    return 0;
}