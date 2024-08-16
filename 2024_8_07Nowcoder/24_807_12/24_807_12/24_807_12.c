#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int n = 0;
    float s = 0;
    float max = 0;
    float min = 100;
    float tmp = 0;
    scanf("%d", &n);
    for (int i = 1; i <= n; i++)
    {
        float j = 0;
        scanf("%f", &j);
        getchar();//吸收掉回车键 
        if (j > max)
        {
            max = j;//如果比上一个大，就进去，反之则不进去
        }
        if (j < min)
        {
            min = j;//如果比上一个小，就进去，反正则不进去
        }
        tmp = j;//用来存放此次循环输入的数据
        s = s + j;//
    }
    printf("%.2f %.2f %.2f", max, min, s / n);


    return 0;
}