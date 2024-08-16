#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

//int main()
//{
//    long num = 0;
//    scanf("%ld", &num);
//    float arr[3] = { 0 };
//    int i = 0;
//    for (i = 0; i <= 2; i++)
//    {
//        scanf("%f", arr + i);
//    }
//    printf("The each subject score of No. %ld is %.2f, %.2f, %.2f.", num, arr[0], arr[1], arr[2]);
//    return 0;
//}
int main()
{
    long num = 0;
    float arr[3] = { 0 };
    float* p1 = arr;
    float* p2 = arr + 1;
    float* p3 = arr + 2;
    scanf("%ld;%f,%f,%f", &num, p1, p2, p3);
    printf("The each subject score of No. %ld is %.2f, %.2f, %.2f.", num, arr[0], arr[1], arr[2]);
    return 0;
}