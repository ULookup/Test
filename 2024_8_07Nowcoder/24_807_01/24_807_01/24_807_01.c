#include <stdio.h>
int main()
{
    const int num = 0;
    int* pn = &num;
    *pn = 20;
    printf("%d", num);
    return 0;
}