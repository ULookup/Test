#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int main()
{
    int arr[3] = { 0 };
    int i = 0;
    for (i = 0; i <= 2; i++)
    {
        scanf("%d", arr + i);
    }
    printf("%d", *(arr + 1));

    return 0;
}