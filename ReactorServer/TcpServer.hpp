#pragma once

#include "LoopThreadPool.hpp"
#include <csignal>

using ConnectedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
using MessageCallback = std::function<void(const std::shared_ptr<Connection>&, Buffer*)>;
using ClosedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
using AnyEventCallback = std::function<void(const std::shared_ptr<Connection>&)>;


class TcpServer
{
public:
    TcpServer(uint16_t port) : 
        _port(port), _next_id(0), _enable_inactive_release(false),
        _baseloop(), _acceptor(&_baseloop, _port), _threadpool(&_baseloop) {
            _acceptor.SetAcceptCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
            _acceptor.Listen();
        }
    /* breif: 设置从属线程数量 */
    void SetThreadCount(int count) { return _threadpool.SetThreadCount(count); }
    /* breif: 启动服务器 */
    void Start() { 
        _threadpool.Create();
        _baseloop.Start(); 
    }
    /* breif: 用户设置回调函数 */
    void SetConnectedCallback(const ConnectedCallback &conncb) { _connected_callback = conncb; }
    void SetMessageCallback(const MessageCallback &msgcb) { _message_callback = msgcb; }
    void SetClosedCallback(const ClosedCallback &closedcb) { _closed_callback = closedcb; }
    void SetAnyEventCallback(const AnyEventCallback &anyeventcb) { _anyevent_callback = anyeventcb; }
    /* breif: 是否启动非活跃连接超时销毁功能 */
    void EnableInactiveRelease(int timeout) {
        _timeout = timeout;
        _enable_inactive_release = true;
    }
    /* breif: 添加定时任务 */
    void RunAfter(const Functor &task, int delay) { _baseloop.RunInLoop(std::bind(&TcpServer::RunAfterInLoop, this, task, delay)); }
    ~TcpServer() = default;
private:
    void RunAfterInLoop(const Functor &task, int delay) {
        _next_id++;
        _baseloop.AddTimer(_next_id, delay, task);
    }
    /* breif: 为新连接构造一个Connection进行管理 */
    void NewConnection(int fd) {
        LOG_DEBUG << "accept a connection, fd = " << fd;
        _next_id++;
        std::shared_ptr<Connection> connection(new Connection(_threadpool.NextLoop(), _next_id, fd));
        LOG_DEBUG << "新建一个 connection";
        connection->SetMessageCallback(_message_callback);
        connection->SetClosedCallback(_closed_callback);
        connection->SetConnectedCallback(_connected_callback);
        connection->SetAnyEventCallback(_anyevent_callback);
        connection->SetSrvClosedCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
        if(_enable_inactive_release) connection->EnableInactiveRelease(_timeout);
        connection->Established();
        _connections[_next_id] = connection;
    LOG_DEBUG << "====THREAD_ID: " << std::this_thread::get_id() << "====";
    }
    void RemoveConnectionInLoop(const std::shared_ptr<Connection> &connection) {
        int id = connection->GetConnID();
        auto it = _connections.find(id);
        if(it != _connections.end()) _connections.erase(it);
    }
    /* breif: 从 _connections 移除信息 */
    void RemoveConnection(const std::shared_ptr<Connection> &connection) {
        _baseloop.RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, connection));
    }
private:
    uint16_t _port;
    uint64_t _next_id; //自动增长ID
    int _timeout; //非活跃连接时间
    bool _enable_inactive_release;
    EventLoop _baseloop;    //主线程，负责监听事件的处理
    Acceptor _acceptor; //监听套接字的管理对象
    LoopThreadPool _threadpool; //从属线程池
    std::unordered_map<uint64_t, std::shared_ptr<Connection>> _connections; //管理所有连接

    /* 这四个回调函数是让服务器模块（组件使用者）来设置的，也是组件使用者使用的 */
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _anyevent_callback;
    /* breif: 组件内的连接关闭回调——组件内设置的，因为webserver组件内会把所有的连接管理起来，一旦某个连接要关闭
    就应该从管理的地方移除掉自己的信息 */
    ClosedCallback _server_closed_callback;
};

class NetWork
{
public:
    NetWork() {
        LOG_DEBUG << "SIGPIPE INIT";
        signal(SIGPIPE, SIG_IGN);
    }
};

static NetWork nw;