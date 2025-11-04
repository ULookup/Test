#include "net/Socket/Socket.h"
#include "lib/IceLog/Logger.h"
#include <iostream>

namespace isframe::net{

    TcpSocket::TcpSocket() noexcept
        : _sockfd(default_sockfd)
        {}

    TcpSocket::TcpSocket(int sockfd) noexcept
        : _sockfd(sockfd)
        {}

    TcpSocket::~TcpSocket() noexcept
    {
        Close();
        LOG_DEBUG << "Socket执行完析构的close()函数";
    }

    void TcpSocket::Create()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(_sockfd < 0){
            //for log...
            exit(static_cast<int>(ExitCode::CREATE_ERROR));
        }
        int opt = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        //for log...
    }

    void TcpSocket::Bind(uint16_t port)
    {
        InetAddress clientaddr(port);
        int ret = bind(_sockfd, (struct sockaddr*)clientaddr.GetSockAddr(), clientaddr.GetSockLen());
        if(ret < 0){
            //for log...
            exit(static_cast<int>(ExitCode::BIN_ERROR));
        }
        //for log...
    }

    void TcpSocket::Listen(int backlog)
    {
        int ret = listen(_sockfd, backlog);
        if(ret < 0){
            //for log...
            exit(static_cast<int>(ExitCode::LISTEN_ERROR));
        }
        //for log...
    }

    std::shared_ptr<Socket> TcpSocket::Accept(InetAddress& clientaddr)
    {
        sockaddr_in peer;
        socklen_t len = sizeof(peer);
        int fd = accept(_sockfd, (struct sockaddr*)&peer, &len);
        if(fd < 0){
            //for log...
            return nullptr;
        }
        //for log...
        clientaddr.SetSockAddr(peer);
        return std::make_shared<TcpSocket>(fd);
    }

    bool TcpSocket::Connect(InetAddress &serveraddr)
    {
        int ret = connect(_sockfd, (struct sockaddr*)serveraddr.GetSockAddr(), serveraddr.GetSockLen());
        if(ret != 0){
            //for log..
            return false;
        }
        else{
            return true;
        }
    }

    ssize_t TcpSocket::Recv(std::string &indata) noexcept
    {
        char buffer[1024];
        LOG_DEBUG << "在系统底层recv()函数处阻塞";
        ssize_t n = recv(_sockfd, buffer, sizeof(buffer) - 1, 0);
        LOG_DEBUG << "系统底层recv()函数收到数据";
        if(n > 0){
            buffer[n] = 0;
            indata += buffer;
        }
        return n;
    }

    ssize_t TcpSocket::SendTo(const std::string &outdata) noexcept
    {
        return send(_sockfd, outdata.c_str(), outdata.size(), 0);
    }

    void TcpSocket::Close() noexcept
    {
        if(_sockfd >= 0){
            close(_sockfd);
            _sockfd = -1;
            LOG_DEBUG << "Socket执行完系统的close()函数";
        }
    }

    void TcpSocket::ShutDown(int how) noexcept
    {
        if(_sockfd > 0){
            shutdown(_sockfd, how);
        }
    }

    int TcpSocket::fd() const noexcept
    {
        return _sockfd;
    }
}