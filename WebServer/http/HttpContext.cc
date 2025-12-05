#include "HttpContext.h"
#include "../util/Util.h"
#include <algorithm>

namespace webserver::http
{
//========== Http 请求行 ==========
/* brief: 接收Http请求行 */
bool HttpContext::RecvHttpLine(src::Buffer *buf) {
    if(_recv_status != RECV_HTTP_LINE) return false; //如果不处于接收请求行状态则返回
    //step 1: 获取一行数据
    std::string line = buf->GetlineAndPop();
    //step 2: 缓冲区数据不足一行/一行超大（即没读到\r\n）
    if(line.size() == 0) {
        // 缓冲区中的数据不足一行，则需要判断缓冲区的可读数据长度，如果很长，则说明有问题
        if(buf->ReadableBytes() > MAX_LINE) {
            _recv_status = RECV_HTTP_ERROR;
            _resp_status = 414; // url too long
            return false;
        }
        // 不足一行，但也不多，就等待新数据到来
        return true;
    }
    // 读到了一行，但一行超大
    if(line.size() > MAX_LINE) {
        _recv_status = RECV_HTTP_ERROR;
        _resp_status = 414; // url too long
        return false;
    }
    // 读取完完整的正常的一行
    bool ret = ParseHttpLine(line);
    if(ret == false) return false;
    _recv_status = RECV_HTTP_HEAD; // 成功接收完请求行，状态切换置接收请求头
    return true;
}
/* brief: 解析Http请求行 */
bool HttpContext::ParseHttpLine(std::string &line) {
    // step 1: 先去掉接收到的请求行的\r\n
    if(line.back() == '\n') line.pop_back();
    if(line.back() == '\r') line.pop_back();
    // 获取解析完成的请求行
    auto matches = util::Util::SplitLine(line);
    assert(matches.size() == 4);
    // step 2: 设置HttpRequest上下文
    // 2.1设置method
    _request._method = matches[0];
    std::transform(_request._method.begin(), _request._method.end(), _request._method, ::toupper);  // 将请求方法转化为大写
    // 2.2设置path
    _request._path = util::Util::UrlDecode(matches[1], false);   // 设置资源路径，需要进行解码操作，但不需要 + 转空格
    // 2.3设置协议版本
    _request._version = matches[3];
    // 2.4设置查询字符串
    std::vector<std::string_view> query_string_arry;
    std::string_view query_string = matches[2];
    // 2.4.1 查询字符串格式位 key1=value1&key2=value2...，先以&分割，得到各个字串
    util::Util::Split(query_string, "&", &query_string_arry);
    // 2.4.2 针对各个字串，以 = 分割，得到key val
    for(auto &line : query_string_arry) {
        size_t pos = line.find("=");
        if(pos == std::string::npos) {
            // 字串里没有 = ，请求行格式出错
            _recv_status = RECV_HTTP_ERROR;
            _resp_status = 400; // BAD REQUEST
            return false;
        }
        // 找到了 = ，对其解码，之后设置进request里
        std::string key = util::Util::UrlDecode(line.substr(0, pos), true);
        std::string val = util::Util::UrlDecode(line.substr(pos + 1), true);
        _request.SetParam(key, val);
    }
}
//========== Http 请求头 ==========
/* brief: 接收Http请求头 */
bool HttpContext::RecvHttpHead(src::Buffer *buf) {
    if(_recv_status != RECV_HTTP_HEAD) return false; // 如果不处于接收请求头状态则返回
    // 一行一行的取出数据，直到遇到空行，头部格式 key1: val1\r\nkey2: val2\r\n...
    while(1) {
        //step 1:获取一行数据
        std::string line = buf->GetlineAndPop();
        //step 2:缓冲区不足一行/一行数据超大
        if(line.size() == 0) {
            //缓冲区中的数据不足一行，则需要判断缓冲区的可读数据长度，如果很长，则有问题
            if(buf->ReadableBytes() > MAX_LINE) {
                _recv_status = RECV_HTTP_ERROR;
                _resp_status = 414; //URL TOO LONG
                return false;
            }
            //不足一行，但不过多，就等新数据到来
            return true;
        }
        if(line.size() > MAX_LINE) {
            //一行数据超大
            _recv_status = RECV_HTTP_ERROR;
            _resp_status = 414; //URL TOO LONG
            return false;
        }
        if(line == "\n" || line == "\r\n") {
            //读到空行，请求头接收完毕
            break;
        }
        // 正常接收到一行数据，对其进行解析
        bool ret = ParseHttpHead(line);
        if(ret == false) {
            return false;
        }
    }
    _recv_status = RECV_HTTP_BODY;
    return true;
}
/* brief: 解析Http请求头 */
bool HttpContext::ParseHttpHead(std::string &line) {
    //step 1: 先去掉\r\n
    if(line.back() == '\n') line.pop_back();
    if(line.back() == '\r') line.pop_back();
    //step 2: 根据": "分割请求头
    size_t pos = line.find(": ");
    if(pos == std::string::npos) {
        // 没有找到分割符
        _recv_status = RECV_HTTP_ERROR;
        _resp_status = 400; // BAD REQUEST
        return false;
    }
    std::string key = line.substr(0, pos);
    std::string val = line.substr(pos + 2);
    _request.SetHeader(key, val);
    return true;
}
//========== Http 请求体 ==========
/* brief: 接收Http请求体 */
bool HttpContext::RecvHttpBody(src::Buffer *buf) {
    if(_recv_status != RECV_HTTP_BODY) return false;
    //step 1: 获取正文长度
    size_t content_length = _request.GetContentLength();
    if(content_length == 0) {\
        // 根据请求头的说明，没有请求体
        _recv_status = RECV_HTTP_OVER;
        return true;
    }
    //step 2:走到这，说明有正文。先明白当前已经接收了多少正文，再计算出要接收的剩余正文
    size_t real_len = content_length - _request._body.size();
    //step 3:接收正文，放到 body 中，要考虑缓冲区的数据是否全是正文
    // 3.1 缓冲区中数据包含所有正文，取出所需数据
    if(buf->ReadableBytes() >= real_len) {
        _request._body.append(buf->ReadPos(), real_len);
        buf->MoveReadOffset(real_len);
        _recv_status = RECV_HTTP_OVER;
        return true;
    }
    // 3.2 走到这说明缓冲区中数据还不包含所有正文，取出后等待下一次正文到来
    _request._body.append(buf->ReadPos(), buf->ReadableBytes());
    buf->MoveReadOffset(buf->ReadableBytes());
    return true;
}

}