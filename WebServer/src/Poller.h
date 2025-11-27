#pragma once

#include "Channel.h"
#include <sys/epoll.h>
#include <unordered_map>

namespace webserver::src
{

#define INITIAL_EPOLLEVENTS 32

class Poller
{
public:
    Poller();

    /* brief: 更新事件监控，添加/更改关心的事件，在对红黑树节点的 events 作修改 */
    void UpdateEvents(Channel *channel);
    /* brief: 移除事件监控 */
    void RemoveEvents(Channel *channel);

    /* brief: 监控事件的执行函数，如果没有事件就绪就会阻塞 */
    void Poll(std::vector<Channel*> &actives);
private:
    /* brief: 更新事件监控的具体实现 */
    void Update(Channel *channel, int op);

    //bool HasChannel(Channel *channel);
private:
    int _epollfd;
    std::vector<struct epoll_event> _events;
    std::unordered_map<int, Channel*> _channels; // 哈希表来存放监控的Channel，用文件描述符作键
};

}