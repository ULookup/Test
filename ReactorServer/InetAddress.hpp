#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <cstring>

class InetAddress
{
public:
    InetAddress() = default;
    InetAddress(uint16_t port, const std::string &ip = "0.0.0.0"){
        memset(&_addr, 0, sizeof(_addr));  //将 _addr 空间清零
        _addr.sin_family = AF_INET;        //设置 _addr 的ip协议为ipv4
        _addr.sin_port = htons(port);      //将主机序 port 转化为网络序
        int ret = inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr.s_addr); //将字符串类型的ip地址转化为点分十进制，再由主机序转为网络序
        if(ret <= 0){
            if(ret == 0){
                //for log...
            }
            else{
                //for log...
            }
        }
    }
    InetAddress(const struct sockaddr_in& addr) { _addr = addr; }

    const sockaddr_in* GetSockAddr() const { return &_addr; }
    const socklen_t GetSockLen() const { return sizeof(_addr); }
    void SetSockAddr(const struct sockaddr_in &addr) { _addr = addr; }

    std::string GetIP(){
        char buffer[INET_ADDRSTRLEN] = {0}; //创建存储ip的缓冲区
        const char *ret = inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof(buffer)); //将ip由网络序转主机序
        if(ret == nullptr){
            //for log...
            return "0.0.0.0";
        }
        return std::string(buffer);
    }
    uint16_t GetPort() { return ntohs(_addr.sin_port); }
    std::string GetIpPort() { return GetIP() + ":" + std::to_string(GetPort()); }

    ~InetAddress() = default;
private:
    struct sockaddr_in _addr;
};