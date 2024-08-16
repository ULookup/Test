#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
int main()
{
    char i = 0;
    while (scanf("%c ", &i) == 1)
    {
        if (i == 'A' || i == 'a' || i == 'E' || i == 'e' || i == 'O' || i == 'o' || i == 'I' || i == 'i' || i == 'U' || i == 'u')
        {
            printf("Vowel\n");
        }
        else
        {
            printf("Consonant\n");
        }
    }

    return 0;
}