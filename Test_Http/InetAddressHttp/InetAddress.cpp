#include "InetAddressHttp/InetAddress.h"

namespace isframe::net{

    isframe::net::InetAddress::InetAddress(uint16_t port, const std::string &ip) noexcept
    {
        memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        int ret = inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr);
        if(ret <= 0){
            if(ret == 0){
                //for log...
            }
            else{
                //for log...
            }
            _addr.sin_addr.s_addr = INADDR_ANY;
        }
    }

    isframe::net::InetAddress::InetAddress(const struct sockaddr_in &addr) noexcept
        : _addr(addr) {}

    std::string isframe::net::InetAddress::toIp() const noexcept
    {
        char buffer[INET_ADDRSTRLEN] = {0};
        const char* ret = inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof(buffer));
        if(ret == nullptr){
            //for log...
            return "0.0.0.0";
        }
        return std::string(buffer);
    }

    uint16_t isframe::net::InetAddress::toPort() const noexcept
    {
        return ntohs(_addr.sin_port);
    }

    std::string isframe::net::InetAddress::toIpPort() const noexcept
    {
        return toIp() + ":" + std::to_string(toPort());
    }
    
}