#include <stdio.h>
void print1(int* arr, int sz)
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
//上文为不关紧要的代码
//下文为核心代码
int cap(const void* e1, const void* e2)
{
	
	if (*(int*)e1 > *(int*)e2)//注意，此处不能直接*e1/*e2，因为它们为void*类型，只能存放，不能访问
		return 1;
	else if (*(int*)e1 < *(int*)e2)
		return -1;
	else
		return 0;
}
void Swap(char* a,char* b,size_t size)
{
	int x = 0;
	for (x = 0; x < size; x++)
	{
		int tmp = 0;
		tmp = *a;
		*a = *b;
		*b = tmp;
		a++;//这里一个字节一个字节的交换，是为了能交换不同的数据类型，因为各数据类型的大小不固定，只能一个字节一个字节逐个交换
		b++;
	}
}
void bubble_qsort(void* base, size_t num, size_t size, int(*cap)(const void* e1, const void* e2))//不确定用户传来什么东西（类型未知），故用void暂时存放地址
{
	int i = 0;
	for (i = 0; i < num - 1; i++)
	{
		int j = 0;
		int flag = 1;
		for (j = 0; j < num - 1; j++)
		{
			if (cap((char*)base+j*size , (char*)base + (j+1)*size)>0)//为适应泛型编程，这里用(char*)base+j*size等价于一个元素一个元素的访问，使得多个类型的数据都可以进行元素间的比较与交换
			{
				Swap((char*)base + j*size, (char*)base + (j + 1)*size,size);
				flag = 0;
			}
		}
		if (flag == 1)//flag用作标记变量，当一趟走过发现没有进行交换，说明已经有序，程序结束
			break;
	}
}
int main()
{
	int arr[] = { 1,3,2,3,1,4,5,2,3,6,7,8,9,1,4,7,9,4,4,2, };
	int sz = sizeof(arr) / sizeof(arr[0]);
	print1(arr, sz);
	bubble_qsort(arr,(size_t)sz,sizeof(arr[0]),cap);
	print2(arr, sz);
	return 0;
}