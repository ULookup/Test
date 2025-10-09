#include<iostream>
#include<unistd.h>
using namespace std;
int main(int argc, char *argv[], char *env[])
{
  static int test = 100;
  pid_t id = fork();
  if(id == 0)
  {
    while(true)
    {
      cout << "我是子进程 pid:" << getpid() << " ppid:" << getppid() << " test:" << test << " &test:" << &test << endl;
      sleep(1);
    }
  }
  else 
  {
    while(true)
    {
      test++;
      cout << "我是父进程 pid:" << getpid() << " ppid:" << getppid() << " test:" << test << " &test:" << &test << endl;
      sleep(1);
    }
  }

  return 0;
}
