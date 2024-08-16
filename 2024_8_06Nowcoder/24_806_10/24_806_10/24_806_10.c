#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
int main()
{
    char a = 0;
    int b = 0;
    float c = 0;
    scanf("%c", &a);
    getchar();
    scanf("%d", &b);
    getchar();
    scanf("%f", &c);
    printf("%c %d %.6f", a, b, c);

    return 0;
}