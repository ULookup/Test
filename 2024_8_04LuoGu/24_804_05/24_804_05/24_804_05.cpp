#include <stdio.h>
#include <string.h>
void swap(char *arr,int j)
{
	char tmp = 0;
	tmp = *(arr + j);
	*(arr + j) = *(arr + (j + 1));
	* (arr + (j + 1)) = tmp;	
}
void swap2(char* arr, int n)
{
	char tmp = 0;
	tmp = *(arr + n);
	*(arr + n) = *(arr + (n + 1));
	*(arr + (n + 1)) = tmp;
}
int main()
{
	char arr2[]="AABCD";
	char arr[] = "AABCD";
	char ARR[] = "BCDAA";
	int sz = sizeof(arr) / sizeof(arr[0]);
	int i = 0;
	int j = 0;
	int m = 0;
	int n = 0;
	int flag = 1;
	for (i = 0; i < sz-2; i++)
	{
		for (j = 0; j < sz-2; j++)
		{
			swap(arr,j);
		}
		if (strcmp(arr, ARR)==0)
		{
			flag = 0;
			printf("%s\n", arr);
			printf("1\n");
		}
	}
	for (m = 0; m < sz - 2; m++)
	{
		for (n = sz-2; n >=0; n--)
		{
			swap2(arr2, n);
		}
		if (strcmp(arr2, ARR) == 0)
		{
			flag = 0;
			printf("%s\n", arr2);
			printf("1\n");
		}
	}
	if(flag==1)
	printf("0\n");
	return 0;
}