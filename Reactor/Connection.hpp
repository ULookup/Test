#pragma once

#include "Reactor.hpp"
#include "InetAddress/InetAddress.h"
#include <string>

class Connection
{
public:
    Connection();
    void Recver()
    {}
    void Sender()
    {}
    void Excepter()
    {}
private:
    int _fd;
    std::string _inbuffer;
    std::string _outbuffer;

    isframe::net::InetAddress _peer;
    Reactor* _owner;
};