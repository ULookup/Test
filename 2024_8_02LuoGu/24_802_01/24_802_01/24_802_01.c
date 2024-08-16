#include <stdio.h>
int main()
{
	int arr[10] = { 0 };
	int* parr1 = &arr[0];
	int* parr2 = arr;
	int* parr3 = &arr;
	printf("%p %p\n", parr1, parr1 + 1);
	printf("%p %p\n", parr2, parr2 + 1);
	printf("%p %p\n", parr3, parr3 + 1);
	printf("%p %p\n", &arr[0], &arr[0] + 1);
	printf("%p %p\n", arr, arr + 1);
	printf("%p %p\n", &arr, &arr+1);

	return 0; 
}