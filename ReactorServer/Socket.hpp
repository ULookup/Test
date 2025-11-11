#pragma once

#include "InetAddress.hpp"
#include <string>
#include <memory>
#include <unistd.h>
#include <fcntl.h>

class Socket
{
public:
    ~Socket() = default;

    virtual bool Create() = 0;
    virtual bool Bind(uint16_t port) = 0;
    virtual bool Listen(int backlog = SOMAXCONN) = 0;
    virtual std::shared_ptr<Socket> Accept(InetAddress &addr) = 0;
    virtual bool Connect(InetAddress &addr) = 0;

    virtual ssize_t Recv(void *buf, size_t len, int flag) = 0;
    virtual ssize_t Send(void *buf, size_t len, int flag) = 0;

    virtual void Close();

    virtual int Fd();
};

class TcpSocket : public Socket
{
public:
    TcpSocket() = default;
    TcpSocket(int sockfd = -1) : _sockfd() {}

    bool Create() override {
        _sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP);
        if(_sockfd < 0) {
            //for log...
            return false;
        }
        return true;
    }

    bool Bind(uint16_t port) override {
        InetAddress addr(port);
        int ret = bind(_sockfd, (struct sockaddr*)addr.GetSockAddr(), addr.GetSockLen());
        if(ret < 0) {
            //for log...
            return false;
        }
        return true;
    }

    bool Listen(int backlog = SOMAXCONN) override {
        int ret = listen(_sockfd, backlog);
        if(ret < 0) {
            //for log...
            return false;
        }
        return true;
    }

    std::shared_ptr<Socket> Accept(InetAddress &clientaddr) override {
        sockaddr_in peer;
        socklen_t len = sizeof(peer);
        int fd = accept(_sockfd, (struct sockaddr*)&peer, &len);
        if(fd < 0) {
            //for log...
            return nullptr;
        }
        clientaddr.SetSockAddr(peer);
        return std::make_shared<TcpSocket>(fd); 
    }

    bool Connect(InetAddress &addr) override {
        int ret = connect(_sockfd, (struct sockaddr*)addr.GetSockAddr(), addr.GetSockLen());
        if(ret < 0) {
            //for log...
            return false;
        }
        return true;
    }

    ssize_t Recv(void *buf, size_t len, int flag = 0) override {
        ssize_t ret = recv(_sockfd, buf, len, flag);
        if(ret <= 0) {
            if(errno == EAGAIN || errno == EINTR) {
                //for log...
                return 0;
            }
            //for log...
            return -1;
        }
        return ret;
    }

    ssize_t NonBlockRecv(void *buf, size_t len) {
        /* brief: MSG_DONTWAIT 表示当前接收为非阻塞*/
        return Recv(buf, len, MSG_DONTWAIT);
    }

    ssize_t Send(void *buf, size_t len, int flag = 0) override {
        ssize_t ret = send(_sockfd, buf, len, flag);
        if(ret < 0) {
            //for log...
            return -1;
        }
        return ret;
    }

    ssize_t NonBlockSend(void *buf, size_t len) {
        /* brief: MSG_DONTWAIT 表示当前接收为非阻塞*/
        return Send(buf, len, MSG_DONTWAIT);
    }

    void Close() {
        if(_sockfd != -1) {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    bool CreateServer(uint16_t port) {
        if(Create() == false) return false;
        if(Bind(port == false)) return false;
        if(Listen() == false) return false;
        SetNonBlock();
        ReuseAddress();
        return true;
    }

    bool CreatClient(uint16_t port, const std::string &ip) {
        InetAddress peer(port, ip);
        if(Create() == false) return false;
        if(Connect(peer) == false) return false;
        return true;
    }

    void ReuseAddress() {
        int opt = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void*)&opt, sizeof(int));
    }
    void SetNonBlock() {
        int flag = fcntl(_sockfd, F_GETFL, 0);
        fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
    }
    
private:
    int _sockfd;
};