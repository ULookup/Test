#include<iostream>
#include<string>
#include<functional>
using namespace std;
using namespace std::placeholders;
template<class T>
T Plus(const T& data1,const T& data2)
{
    return data1 + data2;
}
int main()
{
    function<int(int,int)> Plus_int = Plus<int>;
    function<double(double,double)> Plus_double = [](double x,double y)->double{return x + y;};
    function<string(string)> Plus_string = bind(Plus<string>,_1,"person");
    int a = Plus_int(90,1);
    double b = Plus_double(0.9,0.01);
    string c = Plus_string("sexy");
    cout << a << ' ' << b << ' ' << c << endl;

    return 0;
}
