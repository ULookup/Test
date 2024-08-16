#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int r1 = 0;
    int r2 = 0;
    scanf("%d %d", &r1, &r2);
    float i = ((float)r1 * r2) / (r1 + r2);//只要除法运算有一个运算数为浮点数，那就进行小数运算
    printf("%.1f", i);



    return 0;
}
//#include <stdio.h>
//
//int main()
//{
//    int r1 = 0;
//    int r2 = 0;
//    scanf("%d %d", &r1, &r2);
//    float i = 1.0 / (1.0 / r1 + 1.0 / r2);
//    printf("%.1f", i);
//
//
//
//    return 0;
//}