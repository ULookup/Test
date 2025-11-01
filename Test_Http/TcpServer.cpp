#include "TcpServer.h"

Server::Server(uint16_t port, callback_t cb)
    : _isrunning(false), _cb(cb), _port(port)
    {
        _listensocket = std::make_unique<isframe::net::TcpSocket>(_port);
        _listensocket->Create();
        _listensocket->Bind(_port);
        _listensocket->Listen();
        //std::cout << "[Debug] 服务器被创建并开始监听" << std::endl;
    }

void Server::Run()
{
    //std::cout << "[Debug] 服务器正在运行" << std::endl;
   _isrunning = true;
   while(_isrunning){
        isframe::net::InetAddress clientaddr;
        auto socket = _listensocket->Accept(clientaddr);
        if(socket == nullptr){
            continue;
        }
        //for log...
        //std::cout << "[Debug] 监听并收到客户端请求" << std::endl;
        if(fork() == 0){
            _listensocket->Close();
            HandlerRequest(socket, clientaddr);
            exit(0);
        }
        socket->Close();
   }
   _isrunning = false;
}

void Server::HandlerRequest(std::shared_ptr<isframe::net::Socket> socket, isframe::net::InetAddress addr)
{
    //std::cout << "[Debug] 开始处理客户端请求" << std::endl;
    std::string buffer;
    ssize_t n = socket->Recv(buffer);
    if(n > 0){
        std::string send_str = _cb(buffer);
        socket->SendTo(send_str);
    }
    else if(n == 0){
        //for log..
    }
    else{
        //for log..
    }
    socket->Close();
}