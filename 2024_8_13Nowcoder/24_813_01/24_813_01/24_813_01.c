#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
int main() {
    int year, month;
    scanf("%4d%02d", &year, &month);
    switch (month) {
    case 3:
    case 4:
    case 5: {
        printf("spring");
    }
          break;
    case 6:
    case 7:
    case 8: {
        printf("summer");
    }
          break;
    case 9:
    case 10:
    case 11: {
        printf("autumn");
    }
           break;
    case 12:
    case 1:
    case 2: {
        printf("winter");
    }
          break;
    }
    return 0;
}