#include <stdio.h>
int main()
{
	char i = 0;
	int n = 0;
	char* pi = &i;
	int* pn = &n;
	printf("pn  :%p\n", pn);
	printf("pn+1:%p\n", pn + 1);
	printf("pi  :%p\n", pi);
	printf("pi+1:%p\n", pi + 1);
	

	return 0;
}