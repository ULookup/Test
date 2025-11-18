#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpContext.hpp"
#include "../TcpServer.hpp"

#define DEFAULT_TIMEOUT 30

class HttpServer
{
    using Handler = std::function<void(const HttpRequest&, HttpResponse*)>;
    using Handlers = std::vector<std::pair<std::regex, Handler>>;
public:
    HttpServer(uint16_t port, int timeout = DEFAULT_TIMEOUT) : _server(port) {
        _server.EnableInactiveRelease(timeout);
        _server.SetConnectedCallback(std::bind(&HttpServer::OnConnected, this, std::placeholders::_1));
        _server.SetMessageCallback(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    }
    void SetBaseDir(const std::string &path) { 
        bool ret = Util::IsDirectory(path);
        assert(ret == true);
        _basedir = path;
    }
    void Get(const std::string &pattern, const Handler &handler) {
        _get_route.push_back(std::make_pair(std::regex(pattern), handler));
    }
    void Post(const std::string &pattern, const Handler &handler) {
        _post_route.push_back(std::make_pair(std::regex(pattern), handler));
    }
    void Put(const std::string &pattern, const Handler &handler) {
        _put_route.push_back(std::make_pair(std::regex(pattern), handler));
    }
    void Delete(const std::string &pattern, const Handler &handler) {
        _delete_route.push_back(std::make_pair(std::regex(pattern), handler));
    }
    void SetThreadCount(int count) {
        _server.SetThreadCount(count);
    }
    void Listen() { _server.Start(); }
private:
    void ErrorHandler(const HttpRequest &request, HttpResponse *response) {
        //1.组织错误展示页面
        std::string body;
        body += "<html>";
        body += "<head>";
        body += "<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>";
        body += "</head>";
        body += "<body>";
        body += "<h1>";
        body += std::to_string(response->_status);
        body += " ";
        body += Util::StatusDesc(response->_status);
        body += "</h1>";
        body += "</body>";
        body += "</html>";

        response->SetContent(body, "text/html");
    }
    void WriteResponse(const std::shared_ptr<Connection> &connection, const HttpRequest &request, HttpResponse &response) {
        //LOG_DEBUG << "[WriteResponse] 进入WriteResponse";
        //1. 先完善头部字段
        if(request.IsClose() == true) {
            response.SetHeader("Connection", "close");
        } else {
            response.SetHeader("Connection", "keep-alive");
        }
        if(response._body.empty() == false && response.HasHeader("Content-Length") == false) {
            response.SetHeader("Content-Length", std::to_string(response._body.size()));
        }
        if(response._body.empty() == false && response.HasHeader("Content-Type") == false) {
            response.SetHeader("Content-Type", "application/octet-stream");
        }
        if(response._redirect_flag == true) {
            response.SetHeader("Location", response._redirect_url);
        }
        //2. 将response中的要素，按照http格式进行组织
        std::stringstream rsp_str;
        rsp_str << request._version << " " << std::to_string(response._status) << " " << Util::StatusDesc(response._status) << "\r\n";
        for(auto &head : response._headers) {
            rsp_str << head.first << ": " << head.second << "\r\n";
        }
        rsp_str << "\r\n";
        rsp_str << response._body;
        //LOG_DEBUG << "rsp_str = " << rsp_str.str(); 
        //3. 发送数据
        connection->Send(rsp_str.str().c_str(), rsp_str.str().size());
    }
    bool IsFileHandler(const HttpRequest &request) {
        //LOG_DEBUG << "[IsFileHandler] 进入静态资源判断函数";
        //1. 必须设置了静态资源根目录
        if(_basedir.empty()) return false;
        //2. 请求方法必须是GET、HEAD
        if(request._method != "GET" && request._method != "HEAD") {
            //LOG_DEBUG << "[IsFileHandler] 请求方法不符合静态资源";
            return false;
        }
        //3. 请求的资源路径必须是一个合法路径
        if(Util::ValidPath(request._path) == false) {
            //LOG_DEBUG << "[IsFileHandler] 资源路径不合法: " << request._path;
            return false;
        }
        //4. 请求的资源必须存在，且是普通文件
        std::string request_path = _basedir + request._path; // 为了避免直接修改请求的资源路径
        if(request_path.back() == '/') {
            request_path += "index.html";
        }
        if(Util::IsRegular(request_path) == false) {
            //LOG_DEBUG << "[IsFileHandler] 资源不是普通文件: " << request_path;
            return false;
        }
        //LOG_DEBUG << "[IsFileHandler] 退出静态资源判断函数";
        return true;
    }
    //静态资源请求处理
    void FileHandler(const HttpRequest &request, HttpResponse *response) {
        //LOG_DEBUG << "[FileHandler] 进入FileHandler函数";
        std::string request_path = _basedir + request._path; // 为了避免直接修改请求的资源路径
        //LOG_DEBUG << "[FileHandler] request_path: " << request_path;
        if(request_path.back() == '/') {
            request_path += "index.html";
        }
        bool ret = Util::ReadFile(request_path, &response->_body);
        if(ret == false) {
            return;
        }
        std::string mime = Util::ExtMime(request_path);
        response->SetHeader("Content-Type", mime);
        //LOG_DEBUG << "[FileHandler] 退出FileHandler函数";
        return;
    }
    //功能性请求分类处理
    void Dispatcher(HttpRequest &request, HttpResponse *response, Handlers &handlers) {
        //在对应请求方法的路由表中，查找是否含有对应处理函数，有则调用，没有则返回404
        //思想：路由表存储的是键值对 --- 正则表达式 & 处理函数
        //使用正则表达式，对请求的资源路径进行正则匹配，匹配成功就使用对应函数处理
        //LOG_DEBUG << "[Dispatcher] 对功能性请求进行分类处理";
        for(auto &handler : handlers) {
            const std::regex &re(handler.first);
            const Handler &functor = handler.second;
            bool ret = std::regex_match(request._path, request._matches, re);
            if(ret == false) continue;
            return functor(request, response); //传入请求信息和空response，执行处理函数
        }
        //LOG_DEBUG << "[Dispatcher] 没有找到匹配成功的函数";
        response->_status = 404;
    } 
    void Route(HttpRequest &request, HttpResponse *response) {
        //step 1:对请求进行分辨，是一个静态资源请求，还是一个功能性请求
        // 静态资源请求，则进行静态资源的处理
        // 功能性请求，则需要通过几个请求路由表来确定是否有处理函数
        // GET\HEAD 都先默认是静态资源请求
        if(IsFileHandler(request) == true) {
            return FileHandler(request, response);
        }
        //LOG_DEBUG << "[Route] 开始对非静态资源请求进行路由";
        if(request._method == "GET" || request._method == "") {
            //LOG_DEBUG << "[Route] 方法为GET";
            return Dispatcher(request, response, _get_route);
        } else if(request._method == "POST") {
            //LOG_DEBUG << "[Route] 方法为POST";
            return Dispatcher(request, response, _post_route);
        } else if(request._method == "PUT") {
            //LOG_DEBUG << "[Route] 方法为PUT";
            return Dispatcher(request, response, _put_route);
        } else if(request._method == "DELETE") {
            //LOG_DEBUG << "[Route] 方法为DELETE";
            return Dispatcher(request, response, _delete_route);
        } 

        response->_status = 405; // METHOD NOT ALLOWED
        return;
    }
    //设置上下文
    void OnConnected(const std::shared_ptr<Connection> &connection) {
        connection->SetContext(HttpContext());
        //LOG_DEBUG << "New Connection: " << connection.get();
    }
    void OnMessage(const std::shared_ptr<Connection> &connection, Buffer *buffer) {
        while(buffer->ReadAbleBytes() > 0) {
            //step 1:获取上下文
            HttpContext *context = connection->GetContext()->get<HttpContext>();
            //step 2:通过上下文对缓冲区数据进行解析，得到HttpRequest对象
            // 1.解析出错，直接回复出错响应
            // 2.解析正常，且请求获取完毕，才开始去处理
            context->RecvHttpRequest(buffer);
            HttpRequest &request = context->GetRequest();
            //LOG_DEBUG << "[OnMessage] 获取解析后的 HTTPRequest 对象";
            HttpResponse response(context->GetRespStatus());
            //LOG_DEBUG << "[OnMessage] 构建 Http 应答，并写入状态码";
            if(context->GetRespStatus() >= 400) {
                //进行错误响应，关闭连接
                //LOG_DEBUG << "[OnMessage] 状态码大于 400 进行错误响应";
                ErrorHandler(request, &response); //填充错误显示页面数据到rsp中
                //LOG_DEBUG << "[OnMessage] 组织响应发送给客户端";
                WriteResponse(connection, request, response); //组织响应发送给客户端
                //LOG_DEBUG << "[OnMessage] 发送完毕";
                context->Reset();
                buffer->MoveReadOffset(buffer->ReadAbleBytes()); //出错了就把缓冲区清空
                connection->Shutdown();
                return;
            }
            if(context->GetRecvStatus() != RECV_HTTP_OVER) {
                //当前请求还没有接收完整，等新数据到来在重新继续处理
                //LOG_DEBUG << "[OnMessage] 当前请求还未接收完整，等待新数据到来";
                return;
            }
            //step 3:请求路由 + 业务处理
            //LOG_DEBUG << "[OnMessage] 开始请求路由 + 业务处理";
            Route(request, &response);
            //step 4:对HttpResponse进行组织发送
            WriteResponse(connection, request, response);
            //重置上下文
            context->Reset();
            //step 5:根据长短连接判断是否关闭连接或者继续处理
            if(response.IsClose()) connection->Shutdown();
            //LOG_DEBUG << "[OnMessage] buf readablesize = " << buffer->ReadAbleBytes();
        }
        return;
    }
private:
    Handlers _get_route;
    Handlers _post_route;
    Handlers _put_route;
    Handlers _delete_route;
    std::string _basedir;
    TcpServer _server;
};