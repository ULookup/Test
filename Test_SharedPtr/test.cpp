#include"shared_ptr.h"
#include<iostream>
void test()
{
    Icepop::shared_ptr<int> sp(new int[10]);
    Icepop::shared_ptr<int> sp1(sp);
    Icepop::shared_ptr<int> sp3 = sp1;
    std::cout << "complete" << std::endl;
}
int main()
{
    test();
    return 0;
}
