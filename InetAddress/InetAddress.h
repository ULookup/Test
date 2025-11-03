#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <stdexcept>

/**
 * @file InetAddress.h
 * @brief 网络地址封装结构(IP + Port)的抽象封装
 * 
 * 该类对 ipv4 结构体 'sockaddr_in' 进行了面向对象的封装，
 * 提供了主机(n)/网络(p)字节序转换，字符串格式转换等常用操作。
 * 
 * 主要用于：
 * - 封装客户端、 服务端网络地址信息：
 * - 提供用于 Socket、 Acceptor、 Connector 模块中传递的网络地址信息的包装
 * - 封装底层网络操作接口，减少对 sockaddr_in 的直接操作
 * 
 * 特别说明:
 * - [[nodiscard]] 是 C++17 的新特性，用于标识该函数返回值不可忽略，必须使用。
 * - explicit 标识该函数不支持隐式类型转换（单参数函数），必须显式转换。
 * - noexcept 表明该函数不会抛出异常，在该框架设计的中，仅在逻辑层可以抛出异常，避免执行流混乱。
 * - 该轻量级工具类并未设计线程安全，也不负责内存管理。
 * - 建议在上层被按值传递（或 const 引用传递）。
 * 
 * @author ULookup(icepop)
 * @date 2025-10-30
 * @version 1.1
 * @namespace isframe::net
 */ 

namespace isframe::net{

    class InetAddress
    {
    public:
        InetAddress() noexcept = default;
        /*brief: 用于服务端地址构造*/
        explicit InetAddress(uint16_t port, const std::string &ip = "0.0.0.0") noexcept;
        /*brief: 用于客户端地址构造*/
        explicit InetAddress(const struct sockaddr_in &addr) noexcept;

        /*brief: 获取 sockaddr_in 结构体指针*/
        [[nodiscard]] const sockaddr_in* GetSockAddr() const noexcept { return &_addr; }
        /*brief: 获取 sockaddr_in 结构体大小*/
        [[nodiscard]] const socklen_t GetSockLen() const noexcept { return sizeof(_addr); } 
        /*brief: 更新（设置）新的地址信息*/
        void SetSockAddr(const struct sockaddr_in &addr) noexcept { _addr = addr; }

        /*brief: 将类中 IP 和 Port 的网络序转换为主机序*/
        [[nodiscard]] std::string toIp() const noexcept;
        [[nodiscard]] uint16_t toPort() const noexcept;
        [[nodiscard]] std::string toIpPort() const noexcept;

        ~InetAddress() = default;
    private:
        struct sockaddr_in _addr;
    };
}