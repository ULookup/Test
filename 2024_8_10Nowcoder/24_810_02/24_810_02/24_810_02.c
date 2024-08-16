#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
int is_size(int input)
{
	int fin = input;
	int count = 0;
	while (fin != 0)
	{
		fin = fin / 10;
		count++;
	}
	return count;
}
void exchange(int input,int size)
{
	int fsize = size;
	int flag = 1;
	for (int j = 0; j <= size; j++,fsize--)
	{
		int out = (input / (int)pow(10, fsize))%(int)pow(10,j);
		if (out % 2 == 0)
		{
			int tmp = 0;
			if(flag!=1)
			  printf("%d", tmp);
		}
		else if (out % 2 != 0)
		{
			printf("%d", 1);
			flag = 0;
		}
	}
	if(flag==1)
		printf("%d",0);
}
int main()
{
	int input = 0;
	scanf("%d", &input);
	exchange(input,is_size(input));
	return 0;
}