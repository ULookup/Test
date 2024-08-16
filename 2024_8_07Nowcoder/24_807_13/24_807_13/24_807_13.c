#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
int main()
{
    int a = 0;
    int b = 0;
    int c = 0;
    scanf("%d %d %d", &a, &b, &c);
    float s = (a + b + c) / 2.0;
    float squre = sqrt(s * (s - a) * (s - b) * (s - c));
    printf("circumference=%.2f area=%.2f", (float)(a + b + c), squre);

    return 0;
}