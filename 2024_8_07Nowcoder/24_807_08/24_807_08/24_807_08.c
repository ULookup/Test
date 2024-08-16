#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int X = 0;
    int N = 0;
    scanf("%d %d", &X, &N);
    int i = X + N % 7;
    if (i > 7)
    {
        i = i - 7;
    }
    printf("%d", i);
    return 0;
}