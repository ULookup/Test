#pragma once

#include <string>
#include <unordered_map>

namespace webserver::http
{

class HttpResponse
{
public:
    HttpResponse() : _redirect_flag(false), _status(200) {}
    /* brief: 非正常响应 */
    HttpResponse(int status) : _redirect_flag(false), _status(status) {}
    /* brief: 清空响应上下文 */
    void Reset();
    /* brief: 设置响应头 */
    void SetHeader(const std::string &key, const std::string &val) { _headers.insert(std::make_pair(key, val)); }
    /* brief: 判断是否存在对应响应头 */
    bool HasHeader(const std::string &key) const;
    /* brief: 获取对应响应头 */
    std::string GetHeader(const std::string &key) const;
    /* brief: 设置响应体 */
    void SetContent(const std::string &body, const std::string &type = "text/html");
    /* brief: 设置重定向 */
    void SetRedirect(const std::string &url, int status = 302);
    /* brief: 判断是否是短连接 */
    bool IsClose() const;
public:
    int _status; //响应状态码
    bool _redirect_flag; //是否重定向
    std::string _body;
    std::string _redirect_url; //重定向url
    std::unordered_map<std::string, std::string> _headers; //响应头
};

}