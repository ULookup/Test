#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int n = 0;
    scanf("%d", &n);
    int i = n / 12.;
    printf("%d", i * 4 + 2);

    return 0;
}