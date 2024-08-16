#include <stdio.h>
int* test()
{
	int n = 100;
	return &n;
}
int main()
{
	int* p = test();//p指向的空间被释放了，那块空间可能已经不属于该程序了
	printf("%d\n", *p);
	return 0;
}