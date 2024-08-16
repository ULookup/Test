#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
struct stu
{
	char name[20];
	int age;
	double score;
};
int main()
{
	struct stu s = { "zhangsan",20,85.51 };
	printf("%s %d %lf", s.name, s.age, s.score);
	return 0;

}