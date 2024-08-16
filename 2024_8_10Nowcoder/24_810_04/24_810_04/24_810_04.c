#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    unsigned int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    unsigned int d = 0;
    scanf("%u %u %u %u", &a, &b, &c, &d);
    printf("%d", (a + b - c) * d);
    return 0;
}