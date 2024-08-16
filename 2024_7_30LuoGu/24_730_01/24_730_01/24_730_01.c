#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
size_t strlen_my(char *pa)
{
	int count = 0;
	while (*pa!='\0')
	{
		pa++;
		count++;
	}
	return count;
}
int main()
{
	char arr[] = { "abcdefghijklmn" };
    int* pa = &arr;
	size_t sz=strlen_my(arr);
	printf("%zd", sz);
	return 0;
}