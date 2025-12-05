#include "HttpResponse.h"

namespace webserver::http
{
/* brief: 重置响应上下文 */
void HttpResponse::Reset() {
    _status = 200;
    _redirect_flag = false;
    _body.clear();
    _redirect_url.clear();
    _headers.clear();
}
/* brief: 判断是否存在对应响应头 */
bool HttpResponse::HasHeader(const std::string &key) const {
    auto it = _headers.find(key);
    if(it == _headers.end()) return false;
    return true;
}
/* brief: 获取对应响应头 */
std::string HttpResponse::GetHeader(const std::string &key) const {
    auto it = _headers.find(key);
    if(it == _headers.end()) return "";
    return it->second;
}
/* brief: 设置响应体 */
void HttpResponse::SetContent(const std::string &body, const std::string &type) {
    _body = body;
    SetHeader("Content-Type", type);
}
/* brief: 设置重定向 */
void HttpResponse::SetRedirect(const std::string &url, int status) {
    _status = status;
    _redirect_flag = true;
    _redirect_url = url;
}
/* brief: 判断是否是短链接 */
bool HttpResponse::IsClose() const {
    if(HasHeader("Connection") == true && GetHeader("Connection") == "keep-alive") {
        return false;
    }
    return true;
}

}