#pragma once

#include "LoopThreadPool.h"
#include "Acceptor.h"

namespace webserver::src
{

using ConnectedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
using MessageCallback = std::function<void(const std::shared_ptr<Connection>&, Buffer*)>;
using ClosedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
using AnyEventCallback = std::function<void(const std::shared_ptr<Connection>&)>;


class TcpServer
{
public:
    TcpServer(uint16_t port);
    /* brief: 设置从属线程数量 */
    void SetThreadCount(int count) { return _threadpool.SetThreadCount(count); }
    /* brief: 启动服务器 */
    void Start();
    /* brief: 用户设置回调函数 */
    void SetConnectedCallback(const ConnectedCallback &conncb) { _connected_callback = conncb; }
    void SetMessageCallback(const MessageCallback &msgcb) { _message_callback = msgcb; }
    void SetClosedCallback(const ClosedCallback &clscb) { _closed_callback = clscb; }
    void SetAnyEventCallback(const AnyEventCallback &anyeventcb) { _anyevent_callback = anyeventcb; }
    /* brief: 是否启动非活跃连接超时销毁功能 */
    void EnableInactiveRelease(int timeout);
    /* brief: 添加定时任务 */
    void RunAfter(const Functor &task, int delay) { _baseloop.RunInLoop(std::bind(&TcpServer::RunAfterInLoop, this, task, delay)); }
private:
    void RunAfterInLoop(const Functor &task, int delay);
    /* brief: 为新连接创建一个Connection进行管理 */
    void NewConnection(int fd);
    /* brief: 移除连接的实际执行 */
    void RemoveConnectionInLoop(const std::shared_ptr<Connection> &connection);
    /* brief: 从 _connections 移除连接已经关闭的connection */
    void RemoveConnection(const std::shared_ptr<Connection> &connection) { _baseloop.RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, connection)); }
    
private:
    uint16_t _port;    //该server对应的端口号
    uint64_t _next_id; //自动增长ID
    int _timeout;      //非活跃连接释放时间
    bool _enable_inactive_release; //是否开启非活跃连接释放
    EventLoop _baseloop; //主线程，负责监听事件的处理
    Acceptor _acceptor;  //监听套接字的管理对象
    LoopThreadPool _threadpool; //从属线程池
    std::unordered_map<uint64_t, std::shared_ptr<Connection>> _connections; //管理所有连接

    /* 以下的回调由服务器使用者设置 */
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _anyevent_callback;
    /* 组件内的连接关闭回调，由组件内设置，因为webserver组件会把所有的连接管理起来，一旦某个连接要关闭
    就应该从管理的地方删除掉自己的信息 */
    ClosedCallback _server_closed_callback;
};

}