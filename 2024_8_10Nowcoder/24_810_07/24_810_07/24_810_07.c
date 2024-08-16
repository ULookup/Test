#include <stdio.h>

int main()
{
    int scoure = 0;
    while (scanf("%d", &scoure) == 1 || scanf("%d", &scoure) == 2 || scanf("%d", &scoure) == 3)
    {
        if (scoure >= 60)
            printf("Pass\n");
        else
            printf("Fail\n");
    }

    return 0;
}