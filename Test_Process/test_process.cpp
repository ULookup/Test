#include<stdio.h>
#include<unistd.h>
#include<iostream>
using namespace std;
int main()
{
  printf("我是一个进程 pid:%d , ppid:%d\n",getpid(),getppid());
  sleep(1);
  pid_t id = fork();
try
{
  if(id < 0)
  {
    throw("fork failed!");
  }
  else if(id == 0)
  {
    printf("我是子进程 pid:%d , ppid:%d ,ret:%d\n",getpid(),getppid(),id);
  }
  else
  {
    printf("我是父进程 pid:%d , ppid:%d ,ret:%d\n",getpid(),getppid(),id);
  }
}
catch(const char* msg)
  {
    printf("创建子进程失败!\n");
  }
return 0;
}
