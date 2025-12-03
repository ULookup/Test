#include "TcpServer.h"

namespace webserver::src
{
TcpServer::TcpServer(uint16_t port)
    : _port(port), _next_id(0), _enable_inactive_release(false),
    _baseloop(), _acceptor(&_baseloop, _port), _threadpool(&_baseloop)
    {
        _acceptor.SetAcceptCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
        _acceptor.Listen();
    }
/* brief: 启动服务器 */
void TcpServer::Start() {
    _threadpool.Create();
    _baseloop.Start();
}
/* brief: 开启非活跃连接超时销毁功能 */
void TcpServer::EnableInactiveRelease(int timeout) {
    _timeout = timeout;
    _enable_inactive_release = true;
}
/* brief: 添加定时器实际操作 */
void TcpServer::RunAfterInLoop(const Functor &task, int delay) {
    _next_id++;
    _baseloop.AddTimer(_next_id, delay, task);
}
/* brief: Acceptor的可读事件回调函数 */
void TcpServer::NewConnection(int fd) {
        _next_id++;
        // 构造出一个Connection对象（注：这里可以用内存池优化）
        std::shared_ptr<Connection> connection(new Connection(_threadpool.NextLoop(), _next_id, fd));
        connection->SetMessageCallback(_message_callback);
        connection->SetClosedCallback(_closed_callback);
        connection->SetConnectedCallback(_connected_callback);
        connection->SetAnyEventCallback(_anyevent_callback);
        connection->SetSrvClosedCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
        // 选择是否开启非活跃连接释放
        if(_enable_inactive_release) connection->EnableInactiveRelease(_timeout);
        connection->Established();
        _connections[_next_id] = connection;
    }
/* brief: 移除连接的实际执行操作 */
void TcpServer::RemoveConnectionInLoop(const std::shared_ptr<Connection> &connection) {
    int id = connection->GetConnId();
    auto it = _connections.find(id);
    if(it != _connections.end()) _connections.erase(it);
}

}