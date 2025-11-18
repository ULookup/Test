#pragma once

//#include "../Logger/Logger.h"
#include <string>
#include <unordered_map>

class HttpResponse
{
public:
    HttpResponse() : _redirect_flag(false), _status(200) {}
    HttpResponse(int status) : _redirect_flag(false), _status(status) {}
    void Reset() {
        _status = 200;
        _redirect_flag = false;
        _body.clear();
        _redirect_url.clear();
        _headers.clear();
    }
    void SetHeader(const std::string &key, const std::string &val)  { _headers.insert(std::make_pair(key,val)); }
    bool HasHeader(const std::string &key) const {
        auto it = _headers.find(key);
        if(it == _headers.end()) return false;
        return true;
    }
    std::string GetHeader(const std::string &key) const {
        auto it = _headers.find(key);
        if(it == _headers.end()) return "";
        return it->second;
    }
    void SetContent(const std::string &body, const std::string &type = "text/html") {
        _body = body;
        SetHeader("Content-Type", type);
    }
    void SetRedirect(const std::string &url, int status = 302) {
        _status = status;
        _redirect_flag = true;
        _redirect_url = url;
    }
    bool IsClose() const {
        //LOG_DEBUG << "[IsClose] 进入 IsClose 函数";
        //LOG_DEBUG << "[IsClose] " << GetHeader("Connection"); 
        if(HasHeader("Connection") == true && GetHeader("Connection") == "keep-alive") {
            //LOG_DEBUG << "[IsClose] 判断为长连接";
            return false;
        }
        //LOG_DEBUG << "[IsClose] 判断为短连接";
        return true;
    }
public:
    int _status;
    bool _redirect_flag;
    std::string _body;
    std::string _redirect_url;
    std::unordered_map<std::string, std::string> _headers;
};