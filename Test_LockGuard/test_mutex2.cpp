#include "mutex.h"

using namespace std;

int ticket = 1000;

iceMutex::mutex mutex_1;

void *routine(void *arg)
{
    string name = static_cast<const char *>(arg);
    while (true)
    {
        {
            iceMutex::LockGuard lock(mutex_1);
            if (ticket > 0)
            {
                usleep(1000);
                printf("%s 完成抢票: %d\n", arg, ticket);
                ticket--;
            }
            else
            {
                break;
            }
        }
    }
    return (void *)0;
}

int main()
{

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;
    pthread_t tid4;

    pthread_create(&tid1, nullptr, routine, (void *)"thread-1");
    pthread_create(&tid2, nullptr, routine, (void *)"thread-2");
    pthread_create(&tid3, nullptr, routine, (void *)"thread-3");
    pthread_create(&tid4, nullptr, routine, (void *)"thread-4");

    pthread_join(tid1, nullptr);
    pthread_join(tid2, nullptr);
    pthread_join(tid3, nullptr);
    pthread_join(tid4, nullptr);

    return 0;
}