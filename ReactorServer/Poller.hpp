#pragma once

#include "Channel.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>
#include <cerrno>
#include <unistd.h>

#define MAX_EPOLLEVENTS 1024

class Poller {
public:
    Poller() {
        _epollfd = epoll_create(MAX_EPOLLEVENTS);
        if(_epollfd < 0) {
            LOG_FATAL << "create epoll fail!";
            abort();
        }
    }
    /* brief: 添加或修改监控事件 */
    void UpdateEvents(Channel *channel) {
        bool ret = HasChannel(channel);
        if(ret == false) {
            return Update(channel, EPOLL_CTL_ADD);
        }
        return Update(channel, EPOLL_CTL_MOD);
    }
    /* brief: 移除监控 */
    void RemoveEvents(Channel *channel) {
        auto it = _channels.find(channel->GetFd());
        if(it != _channels.end()) {
            _channels.erase(it);
        }
        Update(channel, EPOLL_CTL_DEL);
    }
    /* brief: 开始监控，返回活跃连接 */
    void Poll(std::vector<Channel*>& active) {
        int nfds = epoll_wait(_epollfd, _evs, MAX_EPOLLEVENTS, -1);
        if(nfds < 0) {
            if(errno == EINTR) {
                return;
            }
            LOG_FATAL << "epoll wait fail!";
            abort();
        }
        for(int i = 0; i < nfds; i++) {
            auto it = _channels.find(_evs[i].data.fd);
            assert(it != _channels.end());
            it->second->SetRevents(_evs[i].events);
            active.push_back(it->second);
        }
    }
private:
    /* brief: 对 epoll 的直接操作 */
    void Update(Channel *channel, int op) {
        int fd = channel->GetFd();
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = channel->GetEvents();
        int ret = epoll_ctl(_epollfd, op, fd, &ev);
        if(ret < 0) {
            LOG_ERROR << "epoll_ctl fail!";
        }
        if (op == EPOLL_CTL_ADD) {
            _channels[fd] = channel;
        }
        return;
    }
    /* brief: 判断一个 Channel 是否已经添加了事件监控 */
    bool HasChannel(Channel *channel) {
        auto it = _channels.find(channel->GetFd());
        if(it == _channels.end()) {
            return false;
        }
        return true;
    }
private:
    int _epollfd;
    struct  epoll_event _evs[MAX_EPOLLEVENTS];
    std::unordered_map<int, Channel*> _channels;
};