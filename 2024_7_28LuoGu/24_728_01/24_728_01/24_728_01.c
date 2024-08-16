#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int main()
{
	int a = 0;
	int b = 0;
	scanf("%d %d", &a, &b);
	printf("交换前:a=%d,b=%d\n", a, b);
	int* pa = &a;
	int* pb = &b;
	printf("a的地址为:%p,b的地址为:%p\n", pa, pb);
	a = a ^ b;
	b = a ^ b;//b=(a^b)^b=a^(b^b)=a^0=a
	a = a ^ b;//a=(a^b)^a(这个a实际上为被赋值后的b，数值上为a
	printf("交换后:a=%d,b=%d\n", a, b);
	return 0;
}