#include<iostream>
#include<unistd.h>
using namespace std;
int main(int argc, char *argv[], char *env[])
{
  for(int i = 0;i < 10; i++)
  {
    cout << "我是超级大帅逼!有" << i << "个小迷妹" << endl;
    sleep(1);
  }

  for(int i = 0;i<argc;i++)
  {
    cout << argv[i] << ' ';
  }
  cout << endl;
  for(int i = 0;env[i];i++)
  {
    cout << env[i] << endl;
  }
  return 0;
}
