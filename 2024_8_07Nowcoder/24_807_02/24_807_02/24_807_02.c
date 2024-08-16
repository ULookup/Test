#include <stdio.h>
int main()
{
	int arr[] = { 1,2,3,4,5,6,7,8,9,10 };
	int* parr = &arr[0];//创建一个指针变量，指向数组的首元素
	int sz = sizeof(arr) / sizeof(arr[0]);//计算数组元素个数
	for (int i = 0; i <= sz - 1; i++)
	{
		printf("%d ", *(parr + i));
	}

	return 0;
}