#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    char input = 0;
    while (scanf("%c", &input) == 1)
    {
        getchar();//用于“吸收”掉回车键产生的一个字符
        printf("%c\n", input + 32);
    }
    return 0;
}