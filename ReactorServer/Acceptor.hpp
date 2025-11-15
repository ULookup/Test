#pragma once

#include "Connection.hpp"
#include "Socket.hpp"

using AcceptCallback = std::function<void(int)>;

class Acceptor
{
public:
    /* attention: 不能将启动读事件监控放在构造里面，必须在设置完成对应回调函数后再启动
    否则有可能启动监控后立即有事件，而处理的时候回调函数还没设置，新连接得不到处理，资源泄露*/
    Acceptor(EventLoop *loop, uint16_t port)
        : _loop(loop), _socket(), _channel(loop, -1)
    {
        bool ret = _socket.CreateServer(port);
        assert(ret == true);
        LOG_DEBUG << "创建监听套接字";
        _channel.SetFd(_socket.Fd());
        _channel.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
        LOG_DEBUG << "channel 设置读事件回调成功";
    }
    void SetAcceptCallback(const AcceptCallback &acptcb) { _accept_callback = acptcb; }
    void Listen() { 
        _channel.EnableRead();
        LOG_DEBUG << "channel 开启对读事件监控";
    }
private:
    /* brief: 监听套接字的读事件回调处理函数——获取新连接，调用_accept_callback函数进行新连接处理 */
    void HandleRead() {
        InetAddress peer;
        std::shared_ptr<Socket> socket = _socket.Accept(peer);
        if(!socket) {
            LOG_DEBUG << "accept socket fail";
            return;
        }
        int newfd = socket->Fd();
        if(newfd < 0) {
            return;
        }
        if(_accept_callback) _accept_callback(newfd);
    }
private:
    TcpSocket _socket;
    EventLoop *_loop;
    Channel _channel;

    AcceptCallback _accept_callback;
};