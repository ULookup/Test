#include<iostream>
#include<unistd.h>
using namespace std;
int main()
{
  pid_t id = fork();
  if(id == 0)
  {
      cout << "我是子进程" << "pid:" << getpid() << endl;
      exit(true);
  }
  while(true)
  {
    cout << "我是父进程" << "pid:" << getpid() << endl;
    sleep(1);
  }
  return 0;
}
