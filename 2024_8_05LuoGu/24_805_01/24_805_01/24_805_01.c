#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
int main()
{
    char password[20] = { 0 };
    printf("请输入密码：");
    scanf("%s", password);
    printf("请确认密码 Yes/No：");
    char a[4] = { 0 };
    scanf("%s", &a);
    char b[4] = "yes";
    if (strcmp(a, b)==0)
    {
        printf("确认成功\n");
    }
    else {
        printf("确认失败\n");
    }
    return 0;
}