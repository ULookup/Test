#include<iostream>
#include<vector>
using namespace std;
int main()
{
  vector<int> arr({0,1,2,3,4,5,6,7,8,9});
  for(auto& e:arr)
  {
    cout << e << ' ';
  }
  cout << endl;
}
