#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int arr[4] = { 0 };
    for (int i = 0; i < 4; i++)
    {
        scanf("%d", &arr[i]);
    }
    int max = 0;
    for (int j = 0; j < 4; j++)
    {
        if (max <= arr[j])
        {
            max = arr[j];
        }
    }
    printf("%d", max);


    return 0;
}