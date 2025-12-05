#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../src/Buffer.h"

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

/* brief: 记录Http请求的接收和处理进度，解决粘包问题 */
class HttpContext
{
public:
    HttpContext() : _resp_status(200), _recv_status(RECV_HTTP_LINE) {}
    /* brief: 重置Http解析上下文 */
    void Reset();
    /* brief: 接收并解析Http请求 */
    bool RecvHttpRequest(src::Buffer *buf);
private:
    //========== Http 请求行 ============
    /* brief: 接收Http请求行 */
    bool RecvHttpLine(src::Buffer *buf);
    /* brief: 解析Http请求行 */
    bool ParseHttpLine(std::string &line);
    //========== Http 请求头 ============
    /* brief: 接收Http请求头 */
    bool RecvHttpHead(src::Buffer *buf);
    /* brief: 解析Http请求头 */
    bool ParseHttpHead(std::string &line);
    //========== Http 请求体 ============
    /* brief: 接收Http请求体 */
    bool RecvHttpBody(src::Buffer *buf);
private:
    int _resp_status;   // 响应状态码
    HttpRecvStatus _recv_status;    //当前接收及解析的阶段状态
    HttpRequest _request;           //已经解析得到的请求信息
};

}