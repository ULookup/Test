#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int age = 0;
    scanf("%d", &age);
    long long sec = age * 3.156e7;
    printf("%lld", sec);
    return 0;
}