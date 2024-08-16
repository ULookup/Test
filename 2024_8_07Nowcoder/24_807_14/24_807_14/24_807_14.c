#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int n = 0;
    int h = 0;
    int m = 0;
    scanf("%d %d %d", &n, &h, &m);
    if (m % h != 0)
        printf("%d", n - (m / h + 1));
    else
        printf("%d", n - (m / h));


    return 0;
}