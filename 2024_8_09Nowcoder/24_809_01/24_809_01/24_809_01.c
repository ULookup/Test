#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int bian(long long n)
{
    int b = n % 10;
    int c = b % 2;
    n = n / 10;
    if (n)
    {

        return 10 * bian(n) + c;
    }

    return c;
}

int main()
{
    long long n = 0;
    scanf("%lld", &n);
    int a = bian(n);
    printf("%d", a);
    return 0;
}