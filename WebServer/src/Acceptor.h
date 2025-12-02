#pragma once

#include "../net/Socket.hpp"
#include "Connection.h"

namespace webserver::src
{

using AcceptCallback = std::function<void(int)>;

/* brief: Acceptor 是一种特殊的Connection，它只负责分配文件描述符/套接字给子线程EventLoop，子线程用它们构造出Connection */
class Acceptor
{
public:
    /* 注：不能将启动读事件监控放在构造里面，理由和 Connection 一样，必须在设置完回调后再启动
        否则有可能构造完后立刻有实际，而此时对应回调函数还没设置，导致得不到处理 */
    /* brief: 构造一个Accptor，只需要告诉它主EventLoop监听的端口即可 */
    Acceptor(EventLoop *loop, uint16_t port);
    /* brief: 暴露给上层，用于设置监听事件回调的函数 */
    void SetAcceptCallback(const AcceptCallback &acptcb) { _accept_callback = acptcb; }
    /* brief: 给上传使用，开始监听（开启对读事件的监控） */
    void Listen() { _channel.EnableRead(); }
private:
    /* brief: 给Accptor管理的底层 channel 设置读事件回调函数。由于 Acceptor 只需要承担分配新连接的工作，所以只需要设置可读事件回调 */
    void HandleRead();
private:
    net::TcpSocket _socket;
    EventLoop *_loop;
    Channel _channel;

    AcceptCallback _accept_callback;
};

}