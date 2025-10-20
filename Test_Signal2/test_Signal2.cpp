#include<iostream>
#include<unistd.h>
#include<cstdio>
#include<sys/types.h>
#include<sys/wait.h>

using namespace std;

void PrintPending(sigset_t &pending)
{
    cout << "#进程[" << getpid() << "] pending: ";
    for(int signo = 31 ; signo >= 1 ; signo--)
    {
        if(sigismember(&pending,signo))
        {
            cout << 1;
        }
        else
        {
            cout << 0;
        }
    }
    cout << endl;
}

void handler(int signo)
{
    cout << signo << " 号信号被递达!" << endl;
    cout << "=========================================" << endl;
    sigset_t pending;
    sigpending(&pending);
    PrintPending(pending);
    cout << "=========================================" << endl;
}

int main()
{
    //0.捕捉2号信号
    signal(2,handler);//自定义捕捉

    //1.屏蔽2号信号
    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigemptyset(&old_set);
    sigaddset(&block_set, SIGINT); //修改进程的屏蔽表（屏蔽SIGINT信号），并没有修改内核

    //1.1设置进入进程的block表中
    sigprocmask(SIG_BLOCK, &block_set, &old_set);//用block_set修改内核的屏蔽表，内核的旧表用old_set保存

    int cnt = 15;
    while(true)
    {
        //2.获取当前进程的pending信号集
        sigset_t pending;
        sigpending(&pending);

        //3.打印pending信号集
        PrintPending(pending);
        cnt--;

        //4.解除对2号信号的屏蔽 
        if(cnt == 0)
        {
            cout << "解除对2号信号的屏蔽!!!" << endl;
            sigprocmask(SIG_SETMASK, &old_set, &block_set);
        }

        sleep(1);
    }
}
