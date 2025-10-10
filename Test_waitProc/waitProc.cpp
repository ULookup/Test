#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>

#define N 10
using namespace std;
int main(int argc, char* argv[], char* env[])
{
  for(int i = 0; i < N; i++)
  {
   pid_t id = fork();
   if(id == 0)
   {
     printf("我是一个子进程 pid:%d, ppid:%d\n",getpid(),getppid());
     sleep(1);
     exit(0);
   }
  }

  printf("正在等待子进程结束ing...\n");
  sleep(3);

  int status = 0;
  for(int i = 0; i < N; i++)
  {
    pid_t rid = waitpid(-1,&status,0);
    printf("等到了一个子进程 pid:%d, status:%d, exit code:%d\n",rid,status,(status>>8)&0xFF);
  }
  return 0;
}
