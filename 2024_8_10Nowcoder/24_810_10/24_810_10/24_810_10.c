#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int a, b, c = 0;
    scanf("%d %d %d", &a, &b, &c);
    if (a <= c && a >= b)
        printf("true");
    else
        printf("false");

    return 0;
}