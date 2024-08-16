#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
long jiecheng(long i)
{
	if (i == 0)
	{
		return 1;
	}
	else
	{
		return i * jiecheng(i - 1);
	}
}
int main()
{
	long i = 0;
	scanf("%d", &i);
	printf("%d", jiecheng(i));
	return 0;
}