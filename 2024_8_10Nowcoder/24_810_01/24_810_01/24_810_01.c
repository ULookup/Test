#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
long Akirid(long a, long b)
{
	long r = 0;
	do
	{
		r = a % b;
		if (r == 0)
			break;
		a = b ;
		b = r;
	} 
	while (r != 0);
	return b;
}
long Anti_akirid(long fa, long fb)
{
	return (fa * fb) / Akirid(fa, fb);
}
int main()
{
	long a = 0;
	long b = 0;
	scanf("%ld %ld", &a, &b);
	long fa = a;
	long fb = b;
	printf("%ld",Akirid(a, b)+Anti_akirid(fa,fb));
	return 0;
}