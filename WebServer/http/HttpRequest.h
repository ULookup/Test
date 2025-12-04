#pragma once

#include <string>
#include <unordered_map>
#include <string_view>

namespace webserver::http
{

class HttpRequest
{
public:
    HttpRequest() : _version("HTTP/1.1") {}
    /* brief: 重置Http请求，防止上下文残留 */
    void Reset();
    /* brief: 设置Http请求请求头 */
    void SetHeader(const std::string &key, const std::string &val) { _headers.insert(std::make_pair(key, val)); }
    /* brief: 判断是否存在指定头部字段 */
    bool HasHeader(const std::string &key) const;
    /* brief: 获取指定头部字段 */
    std::string GetHeader(const std::string &key) const;
    /* brief: 插入查询字符串 */
    void SetParam(const std::string &key, const std::string &val) { _params.insert(std::make_pair(key, val)); }
    /* brief: 判断是否存在指定查询字符串 */
    bool HasParam(const std::string &key) const;
    /* brief: 获取指定查询字符串 */
    std::string GetParam(const std::string &key) const;
    /* brief: 获取请求体大小 */
    size_t GetContentLength() const;
    /* brief: 判断是否是短连接 */
    bool IsClose() const;
public:
    std::string _method;    // Http请求方法
    std::string _path;      // Http请求路径
    std::string _version;   // Http协议版本
    std::string _body;      // Http请求正文
    std::unordered_map<std::string, std::string> _headers; // Http请求头
    std::unordered_map<std::string, std::string> _params;  // Http查询字符串
};

}