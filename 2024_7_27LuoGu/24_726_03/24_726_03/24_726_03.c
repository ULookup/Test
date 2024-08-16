#include <stdio.h>
int main()
{
    int a = 0;
    int b = 0;
    int c = 0;
    scanf("%d %d %d", &a, &b, &c);
    int i = (a + b) / c;
    printf("%d", i);
    return 0;
}