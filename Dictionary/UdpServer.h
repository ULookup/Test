#pragma once
#include <functional>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Logger.h"

#define EXIT_ERROR 1
#define EXIT_FATAL 2

using callback_t = std::function<std::string(const std::string& word, const std::string& whoip, uint16_t& whoport)>;

class DictServer
{
public:
    DictServer(uint16_t port, callback_t cb);

    void Init();

    void Start();

    void Stop();

    ~DictServer() = default;
private:
    int _sockfd;
    uint16_t _port;
    callback_t _cb;
    bool _isrunning;
};