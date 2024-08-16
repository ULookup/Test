//#define _CRT_SECURE_NO_WARNINGS
//#include <stdio.h>
//long GCC(long n, long m)
//{
//	while (m != 0)//m是余数（除数）的位置，当m=0时，说明已经找到了最小公约数
//	{
//		int tmp = m;//辗转相除的核心思想,用余数与除数相除，此时tmp为原来的余数（除开第一次）；
//		m = n % m;  //这个时候“将除数与余数相除的余数作为新的余数”；
//		n = tmp;    //原来的余数成为新的除数；
//	}
//	return m;
//}
//long GCD(long n, long m)
//{
//	for (int i = 0;; i++)
//	{
//		if ((GCC(n, m) * i > n) && (GCC(n, m) > m))
//		{
//			return GCC(n, m) * i;
//		}
//	}
//}
//int main()
//{
//	long n = 0;
//	long m = 0;
//	scanf("%ld %ld", &n, &m);
//	GCC(n, m);
//	GCD(n, m);
//	printf("%ld", GCC(n, m) + GCD(n, m));
//
//	return 0;
//}
#include <stdio.h>

// 欧几里得算法实现
long gcd(long n, long m) {
	while (m != 0) {
		long temp = m;
		m = n % m;
		n = temp;
	}
	return n;
}

// 计算最小公倍数的函数
long lcm(long n, long m) {
	return (n / gcd(n, m)) * m; // 先计算GCD，然后使用公式计算LCM
}

int main() {
	long n = 0;
	long m = 0;
	scanf("%ld %ld", &n, &m);
	long result = gcd(n, m) + lcm(n, m); // 计算GCD和LCM的和
	printf("%ld\n", result);
	return 0;
}