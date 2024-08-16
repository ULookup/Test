#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int main()
{
    int h = 0;
    int r = 0;
    scanf("%d %d", &h, &r);
    float v = (3.14 * r * r * h) / 1000;
    int i = (int)(10 / v);
    if (i < (10 / v))
        printf("%d", (int)(10 / v + 1));
    else
        printf("%d", (int)10 / v);

    return 0;
}