#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>

using namespace std;

int ticket = 1000;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *routine(void *arg)
{
    string name = static_cast<const char*>(arg);
    while(true){
        //pthread_mutex_lock(&lock);
        if(ticket > 0){
            usleep(1000);
            printf("%s 抢到了票:  %d\n",arg,ticket);
            ticket--;
            pthread_mutex_unlock(&lock); 
        }
        else{
            pthread_mutex_unlock(&lock); 
            break;
        }
    }
    return (void*)0;
}

int main()
{
    pthread_t tid1 = 0;
    pthread_t tid2 = 0;
    pthread_t tid3 = 0;
    pthread_t tid4 = 0;

    pthread_create(&tid1, nullptr, routine, (void*)"thread-1");
    pthread_create(&tid2, nullptr, routine, (void*)"thread-2");
    pthread_create(&tid3, nullptr, routine, (void*)"thread-3");
    pthread_create(&tid4, nullptr, routine, (void*)"thread-4");
    
    pthread_join(tid1,nullptr);
    pthread_join(tid1,nullptr);
    pthread_join(tid1,nullptr);
    pthread_join(tid1,nullptr);

    return 0;
}