#include <stdio.h>
#include <stdlib.h>
void print1(int* arr,int sz)
{
	int i = 0;
	printf("排序前:");
	for (i = 0; i < sz - 1; i++)
	{
		printf("%d ", arr[i]);
	}
	printf("\n");
}
void print2(int* arr, int sz)
{
	int i = 0;
	printf("排序后:");
	for (i = 0; i < sz - 1; i++)
	{
		printf("%d ", arr[i]);
	}
}
int cap(const void* e1,const void* e2)
{
	//注意，此处不能直接*e1/*e2，因为它们为void*类型，只能存放，不能访问
	if (*(int*)e1 > *(int*)e2)
		return 1;
	else if (*(int*)e1 < *(int*)e2)
		return -1;
	else
		return 0;
}
int main()
{
	int arr[] = { 1,5,2,5,4,7,8,9,0,6,7,8,1,4,3,5 };
	int sz = sizeof(arr) / sizeof(arr[0]);
	print1(arr,sz);
	qsort(arr, (size_t)sz, sizeof(arr[0]), cap);
	print2(arr, sz);
	
	return 0;
}