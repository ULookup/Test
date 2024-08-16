#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    scanf("%d %d %d %d", &a, &b, &c, &d);
    printf("%.1f", a * 0.2 + b * 0.1 + c * 0.2 + d * 0.5);

    return 0;
}