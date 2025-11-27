#include "Poller.h"
#include <cassert>
#include <cstring>
#include <errno.h>

namespace webserver::src
{

Poller::Poller() : _events(INITIAL_EPOLLEVENTS) 
{
    _epollfd = epoll_create1(EPOLL_CLOEXEC);
    if(_epollfd < 0) {
        //epoll create failed
        abort();
    }
}

void Poller::UpdateEvents(Channel *channel) {
    assert(channel != nullptr);
    int fd = channel->GetFd();
    if(_channels.find(fd) == _channels.end()) {
        // 该连接还没有被 epoll 监控，添加监控
        _channels[fd] = channel;
        return Update(channel, EPOLL_CTL_ADD);
    }
    // 该连接已经被监控了，更新监控信息
    return Update(channel, EPOLL_CTL_MOD);
}

void Poller::RemoveEvents(Channel *channel) {
    assert(channel != nullptr);
    auto it = _channels.find(channel->GetFd());
    if(it != _channels.end()) {
        _channels.erase(it);
    }
    Update(channel, EPOLL_CTL_DEL);
}

void Poller::Poll(std::vector<Channel*> &active) {
    // epoll_wait 等待监控的事件就绪
    int nfds = epoll_wait(_epollfd, _events.data(), static_cast<int>(_events.size()), -1);
    if(nfds < 0) {
        if(errno == EINTR) return;
        //epoll_wait fail!
        abort();
    }

    if(nfds == _events.size()) {
        //事件满了，自动指数扩容
        _events.resize(_events.size() * 2);
    }

    for(int i = 0; i < nfds; ++i) {
        //取出之前放入的channel，并获得revents（就绪的事件），将该channel加入就绪事件队列里
        Channel * channel = static_cast<Channel*>(_events[i].data.ptr);
        assert(channel != nullptr);
        channel->SetRevents(_events[i].events);
        active.push_back(channel);
    }
}
//=============================
//========== private ==========
//=============================

void Poller::Update(Channel *channel, int op) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    // important: 将channel指针交给系统的红黑树，方便后面事件就绪拿到这个指针
    ev.data.ptr = channel;
    // 更新要关心的事件，写入红黑树节点
    ev.events = channel->GetEvents();

    int fd = channel->GetFd();  //拿到channel封装的文件描述符
    int ret = epoll_ctl(_epollfd, op, fd, &ev); //对epoll的红黑树进行操作
    if(ret < 0) {
        if(op == EPOLL_CTL_DEL) {
            //epoll_ctl_del failed
        } else {
            //epoll_ctl failed
        }
    }
}

}