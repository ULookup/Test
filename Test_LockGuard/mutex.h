#ifndef __MUTEX_H__
#define __MUTEX_H__

#define NAME_SIZE 16

#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>

namespace iceMutex
{
    class mutex
    {
    public:
        mutex();
        ~mutex();
        int lock();
        int unlock();

        mutex(const mutex&) = delete;
        mutex& operator=(const mutex&) = delete;

    private:
        pthread_mutex_t _mutex;
    };

    class LockGuard
    {
    public:
        LockGuard(mutex& mutex);
        ~LockGuard();

        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;

    private:
        mutex& _mutex;
        static int g_counter;
    };
}

#endif
