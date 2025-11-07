#pragma once

#include <cstdlib>
#include <sys/epoll.h>

class Epoller
{
public:
    Epoller()
    {
        _epfd = epoll_create(128);
        if(_epfd < 0){
            //for log...
            exit(1);
        }
        //for log...
    }
    void AddEvent();
    int Wait(struct epoll_event revs[], int num, int timeout)
    {
        int n = epoll_wait(_epfd, revs, num, timeout);
        return n;
    }
    ~Epoller();
private:
    int _epfd;
};