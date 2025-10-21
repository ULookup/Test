#include<pthread.h>
#include<iostream>
#include<unistd.h>
#include<string>

using namespace std;

void* thread_routine(void* args)
{
    string threadname = (const char*)args;
    while(true){
        cout << "我是新线程: " << getpid() << " 我叫: " << threadname << endl;
        sleep(1); 
    }
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, nullptr, thread_routine, (void*)"Thread-1");

    while(true){
        cout << "我是主线程: " << getpid() << endl;
        sleep(1);
    }

    return 0;
}