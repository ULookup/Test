#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int i = 0;
    scanf("%d", &i);
    int x = 2;
    x = x << i - 1;
    printf("%d", x);
    return 0;
}