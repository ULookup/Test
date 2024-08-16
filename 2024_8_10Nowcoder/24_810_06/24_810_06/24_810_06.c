#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int is_upper(unsigned int x)
{
	if (x >= 100)
		x = x % 100;
	return x;
}
int main()
{
	unsigned int a = 0;
	unsigned int b = 0;
	scanf("%u %u", &a, &b);
    int afa=is_upper(a);
    int afb=is_upper(b);
	int out = a + b;
	if (out >= 100)
	{
		out = out % 100;
		printf("%d", out);
	}
	else
		printf("%d", out);



	return 0;
}