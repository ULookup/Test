#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
int is_prime(int num)
{
	if (num < 2)
		return 0;
	else if (num == 2)
		return 1;
	else if (num % 2 == 0)
		return 0;
	int i = 0;
	for (i = 3; i <= sqrt(num); i += 2)
	{
		if (num % i == 0)
			return 0;
		else
			return 1;
	}
}
int main()
{
    int i = 0;
    scanf("%d", &i);
    int j = 0;
    j = i-1;
    for (; j > 0; j--)
    {
        if (i% j == 0)
        {
			if (is_prime)
			{
				printf("%d", j);
				break;
			}
        }
    }
    return 0;
}