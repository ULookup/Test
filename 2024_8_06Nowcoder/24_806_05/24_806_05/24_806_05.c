#include <stdio.h>

int main()
{
    long i = 0;
    scanf("%ld", &i);
    printf("year=%ld\n", i / 10000);
    printf("month=%02ld\n", (i / 100) % 100);
    printf("date=%02ld", i % 100);
    return 0;
}