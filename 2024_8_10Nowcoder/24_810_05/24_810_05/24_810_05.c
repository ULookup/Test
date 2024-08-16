#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
int main()
{
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    scanf("%d %d\n%d %d", &x1, &y1, &x2, &y2);
    printf("%d", (int)pow((x1 - x2), 2) + (int)pow((y1 - y2), 2));

    return 0;
}