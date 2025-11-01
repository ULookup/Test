#pragma once

#include "InetAddressHttp/InetAddress.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <memory>

namespace isframe::net{
    
    inline constexpr int default_sockfd = -1;

    enum class ExitCode : uint8_t
    {
        OK = 0,
        CREATE_ERROR,
        BIN_ERROR,
        LISTEN_ERROR
    };

    class Socket
    {
    public:
        virtual ~Socket() noexcept = default;

        virtual void Create() = 0;
        virtual void Bind(uint16_t port) = 0;
        virtual void Listen(int backlog = SOMAXCONN) = 0;
        virtual std::shared_ptr<Socket> Accept(InetAddress &clientaddr) = 0;
        virtual bool Connect(InetAddress &serveraddr) = 0;

        virtual ssize_t Recv(std::string &indata) noexcept = 0;
        virtual ssize_t SendTo(const std::string &outdata) noexcept = 0;
        virtual void Close() noexcept = 0;

         [[nodiscard]] virtual int fd() const noexcept = 0;
    };

    class TcpSocket final : public Socket
    {
    public:
        TcpSocket() noexcept;
        explicit TcpSocket(int sockfd) noexcept;
        ~TcpSocket() noexcept;

        void Create() override;
        void Bind(uint16_t port) override;
        void Listen(int backlog = SOMAXCONN) override;
        std::shared_ptr<Socket> Accept(InetAddress &clientaddr) override;
        bool Connect(InetAddress &serveraddr) override;

        ssize_t Recv(std::string &indata) noexcept override;
        ssize_t SendTo(const std::string &outdata) noexcept override;
        void Close() noexcept override;

        [[nodiscard]] int fd() const noexcept override;
    private:
        int _sockfd;
    };
}