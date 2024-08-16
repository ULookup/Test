#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
int count0(int arr[],int* pa)
{
	int i = 0;
	int count = 0;
	while (i<=20)
	{
		if (*pa == 0)
		{
			count++;
		}
		pa++;
		i++;
	}
	return count;
}
int main()
{
	int arr[] = { 1,2,3,4,0,6,0,1,2,4,6,8,0,0,0,7,6,5,4,0 };
	int* pa = &arr;
	printf("%d", count0(arr, pa));
	return 0;
}