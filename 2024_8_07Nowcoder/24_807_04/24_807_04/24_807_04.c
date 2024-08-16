#include <stdio.h>
size_t strlen_self(char* arr)
{
	char* s =arr ;
	size_t count = 0;
	while (*s != '\0')
	{
		s++;
		count++;
	}
	return count;
}
int main()
{
	char arr[] = "abcdef";
	printf("%zd", strlen_self(arr));
}