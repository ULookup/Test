#include <stdio.h>
//void bubble_sort(int arr[],int sz)
//{
//	int i = 0;
//	for (i = 0; i < sz-1; i++)
//	{
//		int j = 0;
//		int flag = 1;
//		for (j = 0; j < sz-1; j++)
//		{
//			if (arr[j] > arr[j + 1])
//			{
//				int tmp=0;
//			    tmp = arr[j];
//				arr[j] = arr[j + 1];
//				arr[j + 1] = tmp;
//				flag = 0;
//			}
//		}
//		if (flag == 1)
//			break;
//	}
//}
void bubble_sort(int *arr, int sz)
{
	int i = 0;
	for (i = 0; i < sz - 1; i++)
	{
		int j = 0;
		int flag = 1;
		for (j = 0; j < sz - 1; j++)
		{
			if (arr[j] > arr[j + 1])
			{
				int tmp = 0;
				tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
				flag = 0;
			}
		}
		if (flag == 1)
			break;
	}
}
int main()
{
	int arr[13] = { 1,5,7,3,5,2,6,8,9,4,1,4,5, };
	int sz = sizeof(arr) / sizeof(arr[0]);
	bubble_sort(arr,sz);
	int a = 0;
	for (a = 0; a < sz; a++)
	{
		printf("%d ", arr[a]);
	}

	return 0;
}
//arr[x]==*(arr+x)