#pragma once

#include "../net/Socket.hpp"
#include "Buffer.h"
#include "Channel.h"
//#include "../util/Any.hpp" 这里可以用我自己写的 any，谁更好则需要后续来验证
#include <any>

namespace webserver::src 
{

enum ConnectStatus {
    DISCONNECTED,   //已关闭
    CONNECTING,     //连接状态，正在连接的流程中
    CONNECTED,      //已连接，可以通信
    DISCONNECTING   //关闭连接，正在关闭连接的流程中
};

class Connection : public std::enable_shared_from_this<Connection>
{
    /* brief: 以下重命名的类型的函数，是在对应事件发生后的事件处理函数，这里要和 epoll 事件就绪区别开来 */
    using ConnectedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
    using MessageCallback = std::function<void(const std::shared_ptr<Connection>&, Buffer*)>;
    using ClosedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
    using AnyEventCallback = std::function<void(const std::shared_ptr<Connection>&)>; 
public:
    Connection(EventLoop *loop, uint64_t conn_id, int sockfd);
    ~Connection() = default;

    int GetFd() { return _sockfd; }
    int GetConnId() { return _conn_id; }

    /* brief: 判断该连接所处状态 */
    bool IsConnected() { return _status == CONNECTED; }

    /* brief: 设置/获取上下文 */
    void SetContext(const std::any &context) { _context = context; }
    std::any *GetContext() { return &_context; }

    /* brief: 开放给上层设置回调函数的接口 */
    void SetConnectedCallback(const ConnectedCallback &conncb) { _connected_callback = conncb; }
    void SetMessageCallback(const MessageCallback &msgcb) { _message_callback = msgcb; }
    void SetClosedCallback(const ClosedCallback &clscb) { _closed_callback = clscb; }
    void SetAnyEventCallback(const AnyEventCallback &anyeventcb) { _anyevent_callback = anyeventcb; }
    void SetSrvClosedCallback(const ClosedCallback &srvclscb) { _server_closed_callback = srvclscb; }

    /* brief: 建立函数，执行该函数即完成对一个连接的建立 */
    void Established();
    /* brief: 发送数据，需要在对应的 EventLoop线程 内执行 */
    void Send(const char *data, size_t len);

    /* brief: 进入关闭连接流程，需要在对应的 EventLoop线程 内执行 */
    void Shutdown();
    /* brief: 开启非活跃连接销毁，需要在对应的 EventLoop线程 内执行 */
    void EnableInactiveRelease(int sec);
    /* brief: 关闭非活跃连接销毁，需要在对应的 EventLoop线程 内执行 */
    void CancleInactiveRelease();
    /* brief: 协议上下文切换函数，用于更新/切换连接使用的协议。需要在对应的 EventLoop线程 内执行 */
    void Upgrade(const std::any &context,
                const ConnectedCallback &conncb,
                const MessageCallback &msgcb,
                const ClosedCallback &clscb,
                const AnyEventCallback &anyeventcb
            );
private:
    /* brief: 以下 5个 回调函数，都是设置给 Channel 的，用于对应事件就绪后执行 */
    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();
    void HandleEvent();
    /* brief: 关闭并释放连接的函数，不能暴露给外部，需要在对应的 EventLoop线程 内执行 */
    void Release();
    /* brief: 以下函数都是具体执行函数，在对应线程内就执行它们 */
    void EstablishedInLoop();
    void ReleaseInLoop();
    void SendInLoop(Buffer &buf);
    void ShutdownInLoop();
    void EnableInactiveReleaseInLoop(int sec);
    void CancleInactiveReleaseInLoop();
    void UpgradeInLoop(const std::any &context,
                const ConnectedCallback &conncb,
                const MessageCallback &msgcb,
                const ClosedCallback &clscb,
                const AnyEventCallback &anyeventcb);
    
private:
    uint64_t _conn_id;                  // 连接的唯一id，计时器的唯一id也由它标识
    int _sockfd;                        // 该连接管理的套接字文件描述符
    bool _enable_inactive_release;      // 连接是否启动“非活跃连接销毁”的判断标识
    EventLoop *_loop;                   // 管理该连接的 EventLoop
    ConnectStatus _status;              // 连接状态
    net::TcpSocket _socket;             // 该连接管理的套接字
    Channel _channel;                   // 该连接管理的 Channel
    Buffer _in_buffer;                  // 该连接的 输入缓冲区 ，用于存储读事件就绪后 内核socket的接收缓冲区 的数据
    Buffer _out_buffer;                 // 该连接的 输出缓冲区 ，用于存储写事件就绪前，将要转移到 内核socket的发送缓冲区 的数据
    //util::Any _context;
    std::any _context;                  // 存储 应用层协议上下文 的成员

    /* brief: 提供给组件使用者（应用层）的接口（hook），使用者可以设置回调 */
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _anyevent_callback;
    /* brief: 组件内连接关闭回调——组件内设置，因为 webserver 组件内会把所有的连接分配到对应 EventLoop线程 管理起来，一旦某个连接要关闭，就要从管理自己对应
              的 EventLoop线程 内移除自己的信息 */
    ClosedCallback _server_closed_callback;
};

}