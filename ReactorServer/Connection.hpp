#pragma once

#include "Socket.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"
#include "Any.hpp"
#include <cstdint>

#define BUFFER_SIZE 65535

enum ConnectStatus
{
    DISCONNECTED,   //连接关闭状态
    CONNECTING,     //连接建立成功，待处理状态
    CONNECTED,      //连接建立完成，可以通信
    DISCONNECTING   //待关闭状态
};


class Connection : public std::enable_shared_from_this<Connection>
{
    using ConnectedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
    using MessageCallback = std::function<void(const std::shared_ptr<Connection>&, Buffer*)>;
    using ClosedCallback = std::function<void(const std::shared_ptr<Connection>&)>;
    using AnyEventCallback = std::function<void(const std::shared_ptr<Connection>&)>;
public:
    Connection(EventLoop *loop, uint64_t conn_id, int sockfd) :
        _conn_id(conn_id), _sockfd(sockfd), _enable_inactive_release(false),
        _loop(loop), _status(CONNECTING), _socket(sockfd),
        _channel(_loop, _sockfd) 
        {
            _channel.SetCloseCallback(std::bind(&Connection::HandleClose, this));
            _channel.SetEventCallback(std::bind(&Connection::HandleEvent, this));
            _channel.SetReadCallback(std::bind(&Connection::HandleRead, this));
            _channel.SetWriteCallback(std::bind(&Connection::HandleWrite, this));
            _channel.SetErrorCallback(std::bind(&Connection::HandleError, this));
        }
    ~Connection() { LOG_DEBUG << "RELEASE CONNECTION: " << this; }
    int GetFd() { return _sockfd; }
    int GetConnID() { return _conn_id; }
    /* brief: 判断连接是否可通信*/
    bool IsConnected() { return _status == CONNECTED; }
    /* brief: 设置上下文--连接建立完成时调用*/
    void SetContext(const Any &context) { _context = context; }
    /* brief: 获取上下文*/
    Any *GetContext() { return &_context; }
    void SetConnectedCallback(const ConnectedCallback &conncb) { _connected_callback = conncb; }
    void SetMessageCallback(const MessageCallback &msgcb) { _message_callback = msgcb; }
    void SetClosedCallback(const ClosedCallback &closedcb) { _closed_callback = closedcb; }
    void SetAnyEventCallback(const AnyEventCallback &anyeventcb) { _anyevent_callback = anyeventcb; }
    void SetSrvClosedCallback(const ClosedCallback &srvclosedcb) { _server_closed_callback = srvclosedcb; }

    void Established() { _loop->RunInLoop(std::bind(&Connection::EstablishedInLoop, this)); }
    void Send(const char *data, size_t len) { 
        Buffer buf;
        buf.WriteAndPush(data, len);
        _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, std::move(buf))); 
    }
    /* brief: 提供给组件使用者的关闭接口--并不实际关闭，需要判断有没有数据待处理 */
    void Shutdown() { _loop->RunInLoop(std::bind(&Connection::ShutdownInLoop, this)); }
    /* brief: 启动/关闭非活跃销毁，并定义非活跃时长，添加对应计时任务 */
    void EnableInactiveRelease(int sec) { _loop->RunInLoop(std::bind(&Connection::EnableInactiveReleaseInLoop, this, sec)); }
    void CancelInactiveRelease() { _loop->RunInLoop(std::bind(&Connection::CancelInactiveReleaseInLoop, this)); }
    /* brief: 切换协议--重置上下文以及阶段性处理函数 */
    void Upgrade(const Any &context,
                const ConnectedCallback &conncb,
                const MessageCallback &mesgcb,
                const ClosedCallback &closedcb,
                const AnyEventCallback &anyeventcb)
        {
            _loop->AssertInLoop();
            _loop->RunInLoop(std::bind(&Connection::UpgradeInLoop, this, context, conncb, mesgcb, closedcb, anyeventcb));
        }
private:
    /* 五个channel的事件回调函数 */
    /* brief: 描述符可读事件触发后调用的函数 */
    void HandleRead() {
        // step1.接收socket数据，放到缓冲区
        //LOG_DEBUG << "create buf and NonBlockRecv";
        char buf[65536];
        ssize_t ret = _socket.NonBlockRecv(buf, 65535);
        if(ret < 0) {
            LOG_ERROR << "Handleread fail, shutdown connection";
            return ShutdownInLoop(); //先看一下有没有待发送的数据
        }
        //LOG_DEBUG << "data write into buffer";
        _in_buffer.WriteAndPush(buf, ret);
        // step2.调用message_callback进行业务处理
        if(_in_buffer.ReadAbleBytes() > 0) {
            //LOG_DEBUG << "run message_callback";
            return _message_callback(shared_from_this(), &_in_buffer);
        }
    }
    /* brief: 描述符可写事件触发后调用的函数 */
    void HandleWrite() {
        ssize_t ret = _socket.NonBlockSend(_out_buffer.ReadPos(), _out_buffer.ReadAbleBytes());
        if(ret < 0) {
            if(_in_buffer.ReadAbleBytes() > 0) {
                //LOG_DEBUG << "run message_callback";
                _message_callback(shared_from_this(), &_in_buffer); //如果发送数据出错了，就处理完待处理的任务后关闭连接
            }
            return Release(); //实际的关闭释放
        }
        _out_buffer.MoveReadOffset(ret); //将读偏移向后移动
        if(_out_buffer.ReadAbleBytes() == 0) {
            _channel.DisableWrite(); //没有数据待发送就关闭可写事件监控    
            if(_status == DISCONNECTING) return Release(); //如果当前是连接待关闭状态，没有数据就直接关闭
        }

        return;
    }
    /* brief: 描述符关闭事件触发后调用的函数 */
    void HandleClose() {
        if(_in_buffer.ReadAbleBytes() > 0) {
            //LOG_DEBUG << "run message_callback";
            _message_callback(shared_from_this(), &_in_buffer); //触发文件描述符关闭事件，就把没处理的数据处理了
        }
        return Release();
    }
    /* brief: 描述符错误事件触发后调用的函数 */
    void HandleError() { return HandleClose(); }
    /* brief: 描述符任意事件触发后调用的函数 */
    void HandleEvent() {
        //step1. 刷新连接活跃度
        if(_enable_inactive_release)  _loop->RefreshTimer(_conn_id);
        //step2. 调用使用者的任意事件回调
        if(_anyevent_callback) _anyevent_callback(shared_from_this());
    }
    /* brief: 连接获取之后，所处的状态下要进行各种设置（给channel设置事件回调，启动读监控），调用_connected_callback */
    void EstablishedInLoop() {
        //step1. 修改连接状态
        assert(_status == CONNECTING); // 连接当前状态一定是半连接状态
        _status = CONNECTED;
        //一旦启动读事件监控就有可能立即触发读事件，这时候如果启动了非活跃连接销毁就会出错
        //step2. 启动读事件监控
        _channel.EnableRead();
        //LOG_DEBUG << "启动connection读事件监控";
        //step3. 调用回调函数
        if(_connected_callback) _connected_callback(shared_from_this());
    }
    /* brief: 实际释放接口 */
    void ReleaseInLoop() {
        //step1. 修改连接状态，置为DISCONNECTED
        _status = DISCONNECTED;
        //step2. 移除连接的事件监控
        _channel.Remove();
        //step3. 关闭描述符
        _socket.Close();
        //step4. 如果还有定时销毁任务，就取消任务
        if(_loop->HasTimer(_conn_id)) CancelInactiveReleaseInLoop();
        //step5. 调用关闭回调函数（避免先移除服务器的连接管理信息导致Connection释放后的处理）
        if(_closed_callback) _closed_callback(shared_from_this());
        if(_server_closed_callback) _server_closed_callback(shared_from_this()); //移除服务器内部管理的连接信息
    }
    void Release() {
        _loop->PushInLoop(std::bind(&Connection::ReleaseInLoop, this));
    }
    /* brief: 将数据放到缓冲区，启动可写事件监控 */
    void SendInLoop(Buffer &buf) {
        if(_status == DISCONNECTED) return;
        _out_buffer.WriteBufferAndPush(buf);
        if(_channel.WriteAble() == false) _channel.EnableWrite();
    }
    void ShutdownInLoop() {
        _status = DISCONNECTING; //设置连接为半关闭状态
        if(_in_buffer.ReadAbleBytes() > 0) {
            if(_message_callback) _message_callback(shared_from_this(), &_in_buffer);
        }
        //要么就是写入数据的时候出错关闭，要么没有待发送数据，直接关闭
        if(_out_buffer.ReadAbleBytes() > 0) {
            if(_channel.WriteAble() == false) _channel.EnableWrite();
        }
        if(_out_buffer.ReadAbleBytes() == 0) Release();
    }
    void EnableInactiveReleaseInLoop(int sec) {
        //step1. 将判断标识置为true
        _enable_inactive_release = true;
        //step2. 添加/刷新定时销毁任务
        if(_loop->HasTimer(_conn_id)) return _loop->RefreshTimer(_conn_id);
        _loop->AddTimer(_conn_id, sec, std::bind(&Connection::Release, this));
    }
    void CancelInactiveReleaseInLoop() {
        _enable_inactive_release = false;
        if(_loop->HasTimer(_conn_id)) return _loop->CancelTimer(_conn_id);
    }
    void UpgradeInLoop(const Any &context,
                const ConnectedCallback &conncb,
                const MessageCallback &msgcb,
                const ClosedCallback &closedcb,
                const AnyEventCallback &anyeventcb) {
            _context = context;
            _connected_callback = conncb;
            _message_callback = msgcb;
            _closed_callback = closedcb;
            _anyevent_callback = anyeventcb;
        }
private:
    uint64_t _conn_id;  //连接的唯一 ID (TimerID 也用该唯一Id表示)
    int _sockfd;        //连接对应的套接字文件描述符
    bool _enable_inactive_release; //连接是否启动非活跃销毁的判断标识
    EventLoop *_loop;
    ConnectStatus _status;
    TcpSocket _socket;  //套接字操作管理
    Channel _channel;   //连接对应的 Channel
    Buffer _in_buffer;  //输入缓冲区 —— 存放从 socket 中读取到的数据
    Buffer _out_buffer; //输出缓冲区 —— 存放要发送给对端的数据
    Any _context;       //请求的接收处理上下文

    /* 这四个回调函数是让服务器模块（组件使用者）来设置的，也是组件使用者使用的 */
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _anyevent_callback;
    /* breif: 组件内的连接关闭回调——组件内设置的，因为webserver组件内会把所有的连接管理起来，一旦某个连接要关闭
    就应该从管理的地方移除掉自己的信息 */
    ClosedCallback _server_closed_callback;
};