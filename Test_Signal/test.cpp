#include <signal.h>
#include <iostream>

void handler1(int signo)
{
    std::cout << "捕捉到SIGALRM信号:" << signo << std::endl;
    alarm(1);
}
void handler2(int signo)
{
    std::cout << "pid: " << getpid() << " 捕捉到信号: " << signo << std::endl;
}

int main()
{
    signal(SIGALRM, handler1);
    signal(2,handler2);
    alarm(1);

    while(true){
        pause();
    }

    return 0;
}
