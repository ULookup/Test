#include<iostream>
using namespace std;
int sum(int& a,int& b)
{
  int tmp = 0;
  for(int i = a;i<=b;i++)
  {
    tmp = tmp + i;
  }
  return tmp;
}
int main()
{
  int a=5;
  int b=50;
  int result = sum(a,b);
  cout << '[' << a << ',' << b << ']' << '=' << result << endl;
  return 0;
}
