#ifndef __UDP_SERVER_H__
#define __UDP_SERVER_H__

#include "nocopy.hpp"
#include "Logger.h"
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

const static uint16_t defaultport = 8080;
const static int defaultsockfd = -1;

class udp_server : public nocopy
{
public:
    udp_server(const uint16_t& port = defaultport)
        : _port(port), _sockfd(defaultsockfd), _isrunning(false) {}

    void Init()
    {
        //Step 1. Create SocketFd.
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0); // AF_INET --> IPV4. SOCK_DGRAM --> UDP. 
        if(_sockfd < 0){
            LOG_FATAL << "create socket failed";
            exit(1);
        }
        LOG_INFO << "create socket successfully";

        //Step 2. Bind Socket
        //Step 2.1 Fill ip and Port
        struct sockaddr_in local;
        bzero(&local, sizeof(local)); //清空local内数据
        local.sin_family = AF_INET;   //?
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);  //服务端的ip任意都行，只要对应的是同一台主机
        //Step 2.2 Bind with Socket
        int ret = bind(_sockfd, (struct sockaddr*)&local, sizeof(local));
        if(ret < 0){
            LOG_FATAL << "bind socket failed";
            exit(2);
        }
        LOG_INFO << "bind socket successfully: " << _sockfd;
    }

    void Start()
    {
        _isrunning = true;
        while(true){
            char buffer[1024];
            buffer[0] = 0;
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            //1.读取数据
            //LOG_DEBUG << "ready to recv data";
            ssize_t n = recvfrom(_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&peer, &len);
           // LOG_DEBUG << "recvfrom -> buffer:" << buffer;
            if(n > 0){
                uint16_t clientport = ntohs(peer.sin_port);
                std::string clientip = inet_ntoa(peer.sin_addr);

                buffer[n] = 0;
                LOG_DEBUG << "[" << clientip << ":" << clientport << "]#" << buffer;
                
                std::string echo_string = "server echo#";
                echo_string += buffer;

                sendto(_sockfd, echo_string.c_str(), echo_string.size(), 0, (struct sockaddr*)&peer, len);
            }
        }
        _isrunning = false;
    }

    void Stop()
    {
        _isrunning = false;
    }

    ~udp_server(){}
private:
    uint16_t _port;
    int _sockfd;
    bool _isrunning;
};

#endif