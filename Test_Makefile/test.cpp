#include<iostream>
#include<vector>
using namespace std;
int main()
{
  vector<int> arr(10,0);
  for(int i = 0;i<10;i++)
  {
    arr[i] = i;
    cout << arr[i] << ' ';
  }
  cout << endl;
  cout << "Hello,Makefile!" << endl;

  return 0;
}
