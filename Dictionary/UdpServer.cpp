#include "UdpServer.h"

static const int g_defaultSockfd = -1;

DictServer::DictServer(uint16_t port, callback_t cb)
    : _sockfd(g_defaultSockfd), _port(port), _cb(cb), _isrunning(false)
    {}

void DictServer::Init()
{
    _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(_sockfd < 0){
        LOG_FATAL << "create socket failed";
        exit(EXIT_FATAL);
    }
    LOG_INFO << "create socket: " << _sockfd << " successfully";

    struct sockaddr_in local;
    bzero(&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(_port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(_sockfd, (struct sockaddr*)&local, sizeof(local));
    if(ret < 0){
        LOG_FATAL << "bind socket failed";
        exit(EXIT_FATAL);
    }
    LOG_INFO << "bind socket: " << _sockfd << "successfully";
}

void DictServer::Start()
{
    _isrunning = true;
    while(_isrunning){
        char buffer[1024];
        buffer[0] = 0;
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);

        ssize_t n = recvfrom(_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&peer, &len);
        LOG_DEBUG << "recv well";
        if(n > 0){
            buffer[n] = 0;
            uint16_t clientport = ntohs(peer.sin_port);
            std::string clientip = inet_ntoa(peer.sin_addr);

            std::string word = buffer;
            LOG_DEBUG << "用户查找: " << word;

            std::string result = _cb(word, clientip, clientport);
            sendto(_sockfd, result.c_str(), result.size(), 0, (struct sockaddr*)&peer, len);
        }
    }
    _isrunning = false;
}

void DictServer::Stop() { _isrunning = false; }