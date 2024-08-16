#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int i = 0;
    int j = 0;
    scanf("%X %o", &i, &j);
    printf("%d", i + j);

    return 0;
}