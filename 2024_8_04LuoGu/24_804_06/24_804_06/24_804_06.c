#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int jiecheng(int x)
{
	if (x == 0)
		return 1;
	else
        return x * jiecheng(x - 1);
}
int mul(int j, int i)
{
	return jiecheng(i) / (jiecheng(j) * jiecheng(i - j));
}
int main()
{
	int i = 0;//行
	int j = 0;//列
	int n = 0;
	printf("请输入想打印的三角高度:>");
	scanf("%d", &n);
	for (i = 0; i < n-1; i++)
	{
		for (j = 0; j <= i; j++)
		{
			int x = mul(j, i);
			printf("%d ", x);
		}
		printf("%d");
	}
	return 0;
}
//#define _CRT_SECURE_NO_WARNINGS
//#include<stdio.h>
//int jiecheng(int x)
//{
//	if (x == 0)
//		return 1;
//	else
//		return x * jiecheng(x - 1);
//}
//int main()
//{
//	int l = 0;
//	scanf("%d", &l);
//	printf("%d", jiecheng(l));
//}