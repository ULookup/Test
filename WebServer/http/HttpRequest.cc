#include "HttpRequest.h"

namespace webserver::http
{

void HttpRequest::Reset() {
    _method.clear();
    _path.clear();
    _version = "HTTP/1.1";
    _body.clear();
    _headers.clear();
    _params.clear();
}
/* brief: 判断是否存在指定头部字段 */
bool HttpRequest::HasHeader(const std::string &key) const {
    auto it = _headers.find(key);
    if(it == _headers.end()) return false;
    return true;
}
/* brief: 获取指定头部字段 */
std::string HttpRequest::GetHeader(const std::string &key) const {
    auto it = _headers.find(key);
    if(it == _headers.end()) return "";
    return it->second;
}
/* brief: 判断是否存在指定查询字符串 */
bool HttpRequest::HasParam(const std::string &key) const {
    auto it = _params.find(key);
    if(it == _params.end()) return false;
    return true;
}
/* brief: 获取指定查询字符串 */
std::string HttpRequest::GetParam(const std::string &key) const {
    auto it = _params.find(key);
    if(it == _params.end()) return "";
    return it->second;
}
/* brief: 获取请求体大小 */
size_t HttpRequest::GetContentLength() const {
    bool ret = HasHeader("Content-Length");
    if(ret == false) {
        return 0;
    }
    std::string contlen = GetHeader("Content-Length");
    return std::stol(contlen);
}
/* brief: 判断是否是短连接 */
bool HttpRequest::IsClose() const {
    if(HasHeader("Connection") && GetHeader("Connection") == "keep-alive") {
        return false;
    }
    return true;
}

}