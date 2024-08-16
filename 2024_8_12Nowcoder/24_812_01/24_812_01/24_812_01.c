#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int main()
{
    int a = 0;
    scanf("%c", &a);
    if ((a >= 65 && a <= 90) || (a >= 97 && a <= 122))
        printf("YES");
    else
        printf("NO");


    return 0;
}