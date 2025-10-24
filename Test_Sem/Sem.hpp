#ifndef __SEM_H__
#define __SEM_H__

#include <semaphore.h>

namespace icepop
{
    class Sem
    {
    public:
        Sem(int initnum) : _init_num(initnum) { sem_init(&_sem, 0, _init_num); }
        ~Sem() { sem_destroy(&_sem); };

        int P() { return sem_wait(&_sem); };
        int V() { return sem_post(&_sem); };

        Sem(const Sem&) = delete;
        Sem& operator=(const Sem&) = delete; 
    
    private:
        sem_t _sem;
        int _init_num;
    };
}

#endif