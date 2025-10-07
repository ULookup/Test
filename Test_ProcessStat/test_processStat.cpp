#include<unistd.h>
#include<stdio.h>
#include<iostream>
using namespace std;
int main()
{
  for(int i=1;i<=10;i++)
  {
    pid_t id = fork();
    if(id == 0)
    {
      while(1)
      {
        printf("我是子进程%d号 pid:%d ppid:%d ret:%d\n",i,getpid(),getppid(),id);
        sleep(1);
      }
    }
    else
    {
      cout << "子进程被创建 pid:" << id << endl;
    }
    sleep(1);
  }
  while(1)
  {
    printf("我是主进程 pid:%d,ppid:%d\n",getpid(),getppid());
    sleep(1);
  }
  return 0;
}
