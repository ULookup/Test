#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int weight = 0;
    float height = 0;
    scanf("%d %f", &weight, &height);
    float BMI = weight * 1. / (height * height);
    if (BMI <= 23.9 && BMI >= 18.5)
    {
        printf("Normal");
    }
    else
    {
        printf("Abnormal");
    }

    return 0;
}