#include<iostream>
#include<string>
using namespace std;
template<class T>
class Icepop
{
    public:
	Icepop(const T& icepop)
	    :_icepop(icepop)
	{}
	T& get_data()
	{
	    return _icepop;
	}
    private:
	T _icepop;
};
int main()
{   
    Icepop<string>* Lao = new Icepop<string>("LaoIcepop");
    cout << Lao->get_data() << endl;
    return 0;
}
