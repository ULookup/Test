#include<unistd.h>
#include<iostream>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>

#define N 10

using namespace std;
int main(int argc, char* argv[], char* env[])
{
  int i = 0;
  for(;i<N;i++)
  {
    pid_t id = fork();
    if(id == 0)
    {
      int n = 3;
      while(n)
      {
        printf("我是子进程%d,父进程%d,count:%d\n",getpid(),getppid(),n);
        sleep(1);
        n--;
      }
      cout << "子进程退出了!" << endl;
      exit(1);
    }
    cout << "一个进程创建完毕！" << endl;
  }
  cout << "等待子进程结束ing..." << endl;
  sleep(10);
  int status = 0;
  for(int a =0 ;a<10;a++)
  {
    pid_t rpid = waitpid(-1,&status,0);
    printf("等待子进程成功，子进程pid:%d, status: %d, exit code: %d\n",rpid,status,(status>>8)&0xFF);
  }
  return 0;
}
