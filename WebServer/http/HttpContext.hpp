#pragma once

#include "../util/Util.hpp"
#include "../src/Buffer.h"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"

namespace webserver::http
{

#define MAX_LINE 8192

typedef enum {
    RECV_HTTP_ERROR,
    RECV_HTTP_LINE,
    RECV_HTTP_HEAD,
    RECV_HTTP_BODY,
    RECV_HTTP_OVER
} HttpRecvStatus;

/* breif: 记录Http请求的接收和处理进度，解决粘包问题 */
class HttpContext
{
public:
    HttpContext() : _resp_status(200), _recv_status(RECV_HTTP_LINE) {}
    void Reset() {
        _resp_status = 200;
        _recv_status = RECV_HTTP_LINE;
        _request.Reset();
    }
    int GetRespStatus() { return _resp_status; }
    HttpRecvStatus GetRecvStatus() { return _recv_status; }
    HttpRequest &GetRequest() { return _request; }
    /* breif: 接收并解析 Http 请求 */
    void RecvHttpRequest(src::Buffer *buf) {
        switch (_recv_status)
        {
            case RECV_HTTP_LINE: RecvHttpLine(buf);
            case RECV_HTTP_HEAD: RecvHttpHead(buf);
            case RECV_HTTP_BODY: RecvHttpBody(buf);
        }
        //LOG_DEBUG << "[RecvHttpRequest] 接收 Http 请求完毕";
        return;
    }
private:
    bool RecvHttpLine(src::Buffer *buf) {
        if(_recv_status != RECV_HTTP_LINE) return false;
        LOG_DEBUG << "[RecvLine] 开始接收 Http 请求行";
        //step 1:获取一行数据
        std::string line = buf->GetlineAndPop();
        LOG_DEBUG << "[RecvLine]HttpLine: " << line;
        //step 2:缓冲区数据不足一行/一行数据超大
        if(line.size() == 0) {
            //缓冲区中的数据不足一行，则需要判断缓冲区的可读数据长度，如果很长，则有问题
            LOG_DEBUG << "[RecvLine] 缓冲区数据不足一行";        
            if(buf->ReadableBytes() > MAX_LINE) {
                _recv_status = RECV_HTTP_ERROR;
                _resp_status = 414; // "URI TOO LONG"
                LOG_DEBUG << "[RecvLine] 缓冲区数据太长，缓冲区数据错误" << line;        
                return false;
            }
            //不足一行，但也不多，就等新数据到来
            LOG_DEBUG << "[RecvLine] 缓冲区数据不足一行，等待新数据到来";
            return true;
        }
        if(line.size() > MAX_LINE) {
            _recv_status = RECV_HTTP_ERROR;
            _resp_status = 414; // "URI TOO LONG"
            LOG_DEBUG << "[RecvLine] 一行太长，缓冲区数据错误" << line;     
            return false;
        }
        bool ret = ParseHttpLine(line);
        if(ret == false) return false;
        LOG_DEBUG << "[RecvLine] 结束接收请求行";
        _recv_status = RECV_HTTP_HEAD;
        return true;
    }
    bool ParseHttpLine(std::string &line) {
        if(line.back() == '\n') line.pop_back();
        if(line.back() == '\r') line.pop_back();
        auto matches = util::Util::SplitLine(line);
        assert(matches.size() == 4);
        LOG_DEBUG << "[ParseLine] " << "matches[0]: " << matches[0];
        LOG_DEBUG << "[ParseLine] " << "matches[1]: " << matches[1];
        LOG_DEBUG << "[ParseLine] " << "matches[2]: " << matches[2];
        LOG_DEBUG << "[ParseLine] " << "matches[3]: " << matches[3];
        //std::smatch matches;
        //std::regex e("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?", std::regex::icase);
        //bool ret = std::regex_match(line, matches, e);
        //if(ret == false) {
        //    _recv_status = RECV_HTTP_ERROR;
        //    _resp_status = 400; //BAD REQUEST
        //    LOG_DEBUG << "[ParseLine] 正则匹配失败，数据错误";
        //    return false;
        //}
        //请求方法的获取
        _request._method = matches[0];  
        std::transform(_request._method.begin(), _request._method.end(), _request._method.begin(), ::toupper);
        //资源路径的获取，需要进行解码操作，但不需要+转空格
        _request._path = util::Util::UrlDecode(matches[1], false);
        //协议版本的获取
        _request._version = matches[3];
        //查询字符串的获取与处理
        std::vector<std::string_view> query_string_arry;
        std::string_view query_string = matches[2];
        //查询字符串的格式 key=val&key=val..., 先以 & 进行分割,得到各个字串
        util::Util::Split(query_string, "&", &query_string_arry);
        //针对各个字串，以 = 分割，得到key val
        for(auto &line : query_string_arry) {
            size_t pos = line.find("=");
            if(pos == std::string::npos) {
                _recv_status = RECV_HTTP_ERROR;
                _resp_status = 400; //BAD REQUEST
                LOG_DEBUG << "[ParseLine] rsp_status: " <<  _resp_status;
                return false;
            }
            std::string key = util::Util::UrlDecode(line.substr(0, pos), true);
            std::string val = util::Util::UrlDecode(line.substr(pos + 1), true);
            _request.SetParam(key, val);
        }
        return true;
    }
    bool RecvHttpHead(src::Buffer *buf) {
        if(_recv_status != RECV_HTTP_HEAD) return false;
        //一行一行取出数据，直到遇到空行，头部格式 key: val\r\nkey: val\r\n
        LOG_DEBUG << "[RecvHead] 开始接收 Http 请求头";
        while(1) {
            //step 1:获取一行数据
            std::string line = buf->GetlineAndPop();
            LOG_DEBUG << "[RecvHead] " << line;
            //step 2:缓冲区数据不足一行/一行数据超大
            if(line.size() == 0) {
                //缓冲区中的数据不足一行，则需要判断缓冲区的可读数据长度，如果很长，则有问题
                if(buf->ReadableBytes() > MAX_LINE) {
                    _recv_status = RECV_HTTP_ERROR;
                    _resp_status = 414; // "URI TOO LONG"
                    LOG_DEBUG << "[RecvHead] 缓冲区数据太长，缓冲区数据错误";
                    return false;
                }
                //不足一行，但也不多，就等新数据到来
                LOG_DEBUG << "[RecvHead] 缓冲区数据不足一行，等待新数据到来";
                return true;
            }
            if(line.size() > MAX_LINE) {
                _recv_status = RECV_HTTP_ERROR;
                _resp_status = 414; // "URI TOO LONG"
                LOG_DEBUG << "[RecvHead] 一行太长，缓冲区数据错误";
                return false;
            }
            if(line == "\n" || line == "\r\n") {
                LOG_DEBUG << "[RecvHead] 读到空行，请求头接收完毕";
                break;
            }
            bool ret = ParseHttpHead(line);
            if(ret == false) {
                LOG_DEBUG << "[RecvHead] 该行请求头解析失败: " << line;
                LOG_DEBUG << "[RecvHead] 结束接收请求头";
                return false;
            }
        }
        LOG_DEBUG << "[RecvHead] 结束接收请求头";
        _recv_status = RECV_HTTP_BODY;
        return true;
    }
    bool ParseHttpHead(std::string &line) {
        if(line.back() == '\n') line.pop_back();
        if(line.back() == '\r') line.pop_back();
        size_t pos = line.find(": ");
        if(pos == std::string::npos) {
            _recv_status = RECV_HTTP_ERROR;
            _resp_status = 400; // "BAD REQUEST"
            LOG_DEBUG << "[ParseHead] 没有找到 : 分割， 无效请求头";
            return false;
        }
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 2); //这里的2是指": "的长度
        _request.SetHeader(key, val);
        return true;
    }
    bool RecvHttpBody(src::Buffer *buf) {
        if(_recv_status != RECV_HTTP_BODY) return false;
        LOG_DEBUG << "[RecvBody] 开始接收 Http 请求体";
        //step 1:获取正文长度
        size_t content_length = _request.GetContentLength();
        LOG_DEBUG << "[RecvBody] content-length: " << content_length;
        if(content_length == 0) {
            LOG_DEBUG << "[RecvBody] 正文长度为 0 , 接收 Http 请求完毕";
            _recv_status = RECV_HTTP_OVER;
            return true;
        }
        //step 2:当前已经接收了多少正文 _request._body
        size_t real_len = content_length - _request._body.size();
        LOG_DEBUG << "[RecvBody] real_len = " << real_len;
        LOG_DEBUG << "[RecvBody] ReadAbleBytes = " << buf->ReadableBytes();
        //step 3:接收正文，放到 body 中，要考虑缓冲区中的数据是否是全部正文
        // 3.1 缓冲区中数据，包含所有正文，取出所需数据
        if(buf->ReadableBytes() >= real_len) {
            _request._body.append(buf->ReadPos(), real_len);
            LOG_DEBUG << "[RecvBody] real_len = " << real_len;
            buf->MoveReadOffset(real_len);
            _recv_status = RECV_HTTP_OVER;
            LOG_DEBUG << "[RecvBody] 缓冲区数据满足正文需要，取出需要的数据";
            return true;
        }
        // 3.2 缓冲区中数据，不足正文需要，取出数据，然后等待新数据到来
        _request._body.append(buf->ReadPos(), buf->ReadableBytes());
        buf->MoveReadOffset(buf->ReadableBytes());
        LOG_DEBUG << "[RecvBody] ReadAbleBytes = " << buf->ReadableBytes();
        LOG_DEBUG << "[RecvBody] 缓冲区数据不足正文需要，取出数据等待新数据到来";
        return true;
    }
private:
    int _resp_status; //响应状态码
    HttpRecvStatus _recv_status; //当前接收及解析的阶段状态
    HttpRequest _request; //已经解析得到的请求信息
}; 

}