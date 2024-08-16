#include <stdio.h>
int main()
{
	int i = 0;
	int* pi = &i;
	printf("%p\n", pi);
	printf("%d\n", *pi);
	return 0;
}