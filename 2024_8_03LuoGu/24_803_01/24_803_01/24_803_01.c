#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int plu(int x, int y)
{
	return x + y;
}
int min(int x, int y)
{
	return x - y;
}
int mul(int x, int y)
{
	return x * y;
}
int div(int x, int y)
{
	return x / y;
}
void Calc(int(*pf)(int,int),int x,int y)
{
	printf("请输入操作数:>\n");
	scanf("%d %d", &x, &y);
	int ret = pf(x, y);
	printf("%d\n", ret);
}
void menu(void)
{
	printf("****************\n");
	printf("******1.plu*****\n");
	printf("******2.min*****\n");
	printf("******3.mul*****\n");
	printf("******4.div*****\n");
	printf("******0.exit****\n");
	printf("****************\n");
}
int main()
{
	int input = 0;
	int x = 0;
	int y = 0;
	do
	{
		menu();
		printf("请选择:>\n");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			Calc(plu, x, y);
			break;
		case 2:
			Calc(min, x, y);
			break;
		case 3:
			Calc(mul, x, y);
			break;
		case 4:
			Calc(div, x, y);
			break;
		case 0:
			printf("退出计算器\n");
			break;
		}
	} 
	while (input);
	return 0;
}