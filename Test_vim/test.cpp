#include<iostream>
#include<vector>
using namespace std;
int main()
{
    vector<int> a = {0,1,2,3,4,5,6};
    for(auto& e: a)
    {
        cout << e << ' ';
    }
    cout << endl;
    return 0;
}
