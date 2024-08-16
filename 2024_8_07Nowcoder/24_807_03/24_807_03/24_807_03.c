#include <stdio.h>
int main()
{
	int arr[10] = { 0 };
	printf("%2d\n", &arr[9] - &arr[0]);
	printf("%2d\n", &arr[0] - &arr[9]);

	return 0;
}