#include<iostream>
#include<string>
template<class T>
T Test(const T& t)
{
  return t;
}
int main()
{
  std::string a = "Hello,Git!";
  std::cout << Test<std::string>(a) << std::endl;
  return 0;
}
