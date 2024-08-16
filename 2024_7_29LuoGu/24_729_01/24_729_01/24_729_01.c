#include <stdio.h>
int main()
{
	int arr[10] = { 1,2,3,4,5,6,7,8,9,10 };
	int i = sizeof(arr) / sizeof(arr[0]);
	int count = 0;
	int* j = &arr;
	for (; count < i; count++)
	{
		printf("%d ", *(j + count));
	}
	return 0;
}