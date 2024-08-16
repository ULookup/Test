#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int plu(int i)
{
    if (i == 1)
    {
        return 1;
    }
    else
        return i + plu(i - 1);
}
int main()
{
    int i = 0;
    scanf("%d", &i);
    printf("%d", plu(i));
    return 0;
}
//递归求解
//int main()
//{
//    int i = 0;
//    long s = 0;
//    scanf("%d", &i);
//    for (; i > 0; i--)
//    {
//        s = s + i;
//    }
//    printf("%ld", s);
//    return 0;
//}
//循环求解