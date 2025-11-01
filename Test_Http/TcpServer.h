#pragma once

#include "InetAddressHttp/InetAddress.h"
#include "SocketHttp/Socket.h"
#include <iostream>
#include <functional>

using callback_t = std::function<std::string(std::string& buffer)>;

class Server
{
public:
    Server(uint16_t port, callback_t cb);

    void Run();

    void HandlerRequest(std::shared_ptr<isframe::net::Socket> socket, isframe::net::InetAddress addr);

    ~Server() = default;
private:
    bool _isrunning;
    callback_t _cb;
    int _port;
    std::unique_ptr<isframe::net::Socket> _listensocket;
};