#pragma once

//#include "../Logger/Logger.h"
#include <string>
#include <regex>
#include <unordered_map>

class HttpRequest
{
public:
    HttpRequest() : _version("HTTP/1.1") {}
    /* breif: 重置请求，防止上下文残留 */
    void Reset() {
        _method.clear();
        _path.clear();
        _version = "HTTP/1.1";
        _body.clear();
        std::smatch match;
        _matches.swap(match);
        _headers.clear();
        _params.clear();
    }
    /* brief: 插入头部字段 */
    void SetHeader(const std::string &key, const std::string &val) { _headers.insert(std::make_pair(key,val)); }
    /* brief: 判断是否存在指定头部字段 */
    bool HasHeader(const std::string &key) const {
        auto it = _headers.find(key);
        if(it == _headers.end()) return false;
        return true;
    }
    /* brief: 获取指定头部字段 */
    std::string GetHeader(const std::string &key) const {
        auto it = _headers.find(key);
        if(it == _headers.end()) return "";
        return it->second;
    }
    /* brief: 插入查询字符串 */
    void SetParam(const std::string &key, const std::string &val) { _params.insert(std::make_pair(key,val)); }
    /* brief: 判断是否存在指定查询字符串 */
    bool HasParamr(const std::string &key) const {
        auto it = _params.find(key);
        if(it == _params.end()) return false;
        return true;
    }
    /* brief: 获取指定查询字符串 */
    std::string GetParam(const std::string &key) const {
        auto it = _params.find(key);
        if(it == _params.end()) return "";
        return it->second;
    }
    /* brief: 获取请求体大小 */
    size_t GetContentLength() const {
        bool ret = HasHeader("Content-Length");
        if(ret == false) {
            return 0;
        }
        std::string contlen = GetHeader("Content-Length");
        return std::stol(contlen);
    }
    /* brief: 判断是否是短连接*/
    bool IsClose() const{
        //LOG_DEBUG << "[IsClose] 进入 IsClose 函数";
        //LOG_DEBUG << "[IsClose] " << GetHeader("Connection"); 
        if(HasHeader("Connection") && GetHeader("Connection") == "keep-alive") {
            //LOG_DEBUG << "[IsClose] 判断为长连接";
            return false;
        }
        //LOG_DEBUG << "[IsClose] 判断为短连接";
        return true;
    }
public:
    std::string _method;    //Http请求方法
    std::string _path;      //Http请求的资源路径
    std::string _version;   //Http协议版本
    std::string _body;      //Http请求正文
    std::smatch _matches;   //资源路径的正则提取数据
    std::unordered_map<std::string, std::string> _headers;  //头部字段
    std::unordered_map<std::string, std::string> _params;   //查询字符串
};