#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int seconds = 0;
    printf("请输入秒数:>");
    scanf("%d", &seconds);
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int se = (seconds % 3600) % 60;
    printf("%d3时%3d分%3d秒", hours, minutes, se);
    return 0;
}