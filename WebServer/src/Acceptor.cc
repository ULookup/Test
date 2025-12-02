#include "Acceptor.h"

namespace webserver::src
{

Acceptor::Acceptor(EventLoop *loop, uint16_t port) 
    : _loop(loop), _socket(), _channel(loop, -1)
    {
        bool ret = _socket.CreateServer(port);
        assert(ret == true);
        _channel.SetFd(_socket.Fd());
        _channel.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
    }

void Acceptor::HandleRead() {
    net::InetAddress peer;  // 创建一个网络地址对象，用于存储对端地址
    std::shared_ptr<net::Socket> socket = _socket.Accept(peer); // 接收连接
    if(!socket) {
        // accept socket failed
        return;
    }
    int newfd = socket->Fd();
    if(newfd < 0) {
        return;
    }
    if(_accept_callback) _accept_callback(newfd);
}

}