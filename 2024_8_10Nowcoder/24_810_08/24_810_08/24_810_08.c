#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int x = 0;
    while (scanf("%d", &x) > 0)
    {
        if (x % 2 == 0)
            printf("Even\n");
        else if (x % 2 != 0)
            printf("Odd\n");
    }
    return 0;
}