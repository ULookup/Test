#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    double f = 0;
    scanf("%lf", &f);
    double c = 5.0 / 9 * (f - 32);
    printf("%.3f", c);


    return 0;
}