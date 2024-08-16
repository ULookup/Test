#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
    int hour = 0;
    int minute = 0;
    long sleep = 0;
    scanf("%d:%d %ld", &hour, &minute, &sleep);
    int fuhour = sleep / 60 + hour;
    int fuminute = sleep % 60 + minute;
    if (fuminute > 60)
    {
        fuhour = fuhour + fuminute / 60;
        fuminute = fuminute % 60;
    }
    if (fuhour > 24)
        fuhour = fuhour % 24;
    printf("%02d:%02d", fuhour, fuminute);


    return 0;
}
