#include <stdio.h>
#include <math.h>
int dec(int i)
{
    int j = 0;
    int flag = 0;
    for (j = 3; j <= sqrt(i); j = j + 2)
    {
        if (i % j == 0)
        {
            flag++;
        }
    }
    if (flag == 0)
        return 1;
    else
        return 0;
}
int main()
{
    int i = 999;
    int count = 0;
    for (; i > 99; i = i - 2)
    {
        if (dec(i))
        {
            count++;
        }
    }
    printf("%d", count);
    return 0;
}