#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int Fact(int input)
{
	if (input == 0)
		return 1;
	else
		return input * Fact(input - 1);
}
int main()
{
	int i = 0;
	scanf("%d", &i);
	int j=Fact(i);
	printf("%d", j);
	return 0;
}