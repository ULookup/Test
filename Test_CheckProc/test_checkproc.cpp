#include<iostream>
#include<unistd.h>
using namespace std;
int main()
{
  while(1)
  {
    cout << "pid:" << getpid() << endl;
    sleep(2);
  }
  return 0;
}
