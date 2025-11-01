#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>

static const std::string linesep = "\r\n";
static const std::string innersep1 = " ";
static const std::string innersep2 = ": ";
static const std::string webroot = "./wwwroot";
static const std::string defaulthome = "index.html";
static const std::string html_404 = "404.html";
static const std::string suffixsep = ".";
static const std::string argssep = "?";

class HttpRequest
{
private:
    std::string ReadOneLine(std::string &reqstr, bool *status)
    {
        auto pos = reqstr.find(linesep);
        if (pos == std::string::npos)
        {
            *status = false;
            return std::string();
        }
        *status = true;
        auto line = reqstr.substr(0, pos);
        reqstr.erase(0, pos + linesep.size());
        return line;
    }

    void ParseReqLine(std::string &reqline)
    {
        std::stringstream ss(reqline);
        ss >> _method >> _uri >> _httpversion;
    }

    void BuildKV(std::string &reqline, std::string *K, std::string *V)
    {
        auto pos = reqline.find(innersep2);
        if (pos == std::string::npos)
        {
            *K = *V = std::string();
            return;
        }
        *K = reqline.substr(0, pos);
        *V = reqline.substr(pos + innersep2.size());
    }

public:
    HttpRequest() noexcept = default;
    ~HttpRequest() noexcept = default;
    void Serialize() { /* 不需要对请求序列化 */ }
    bool Deserialize(std::string &reqstr)
    {
        bool status = true;
        std::string reqline = ReadOneLine(reqstr, &status);
        if (!status)
        {
            return false;
        }

        std::cout << "##############" << reqline << "##############" << std::endl;

        ParseReqLine(reqline);

        while (true)
        {
            status = true;
            reqline = ReadOneLine(reqstr, &status);
            if (status && !reqline.empty())
            {
                std::string k, v;
                BuildKV(reqline, &k, &v);
                if (k.empty() || v.empty())
                {
                    continue;
                }
                _req_headers.insert(std::make_pair(k, v));
            }
            else if (status)
            {
                _blank_line = linesep;
                break;
            }
            else
            {
                std::cout << "非法请求" << std::endl;
                break;
            }
        }

        _path = webroot;
        _path += _uri;

        if (_uri == "/")
        {
            _path += defaulthome;
        }
        std::cout << "_path: " << _path << std::endl;
        if (_method == "GET")
        {
            auto pos = _path.find(argssep);
            if (pos != std::string::npos)
            {
                _req_body = _path.substr(pos + argssep.size());
                _path = _path.substr(0, pos);
            }
        }
        else if (_method == "POST")
        {
            _req_body = reqstr;
        }

        return true;
    }

    std::string GetPath()
    {
        return _path;
    }

    void SetPath(const std::string &path)
    {
        _path = path;
    }

    std::string Suffix()
    {
        if (_path.empty())
        {
            return std::string();
        }
        else
        {
            auto pos = _path.rfind(suffixsep);
            if (pos == std::string::npos)
            {
                return std::string();
            }
            else
            {
                return _path.substr(pos);
            }
        }
    }

private:
    std::string _method;
    std::string _uri;
    std::string _httpversion;
    std::unordered_map<std::string, std::string> _req_headers;
    std::string _blank_line;
    std::string _req_body;

    std::string _path;
};

class HttpResponse
{
private:
    std::string CodeToDesc(int code)
    {
        switch (code)
        {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 301:
            return "Moved Permanently";
        case 302:
            return "See Other";
        case 307:
            return "Temporary Redirect";
        default:
            return "UnKown";
        }
    }

public:
    HttpResponse() : _httpversion("HTTP/1.1"), _blank_line(linesep) {}
    ~HttpResponse() = default;

    std::string Serialize()
    {
        std::string respstr = _httpversion + innersep1 + std::to_string(_code) + innersep1 + _desc + linesep;
        if (!_resp_body.empty())
        {
            std::string len = std::to_string(_resp_body.size());
            SetHeader("Content-Length", len);
        }

        for (auto &elem : _resp_headers)
        {
            std::string line = elem.first + innersep2 + elem.second + linesep;
            respstr += line;
        }

        respstr += _blank_line;
        respstr += _resp_body;

        return respstr;
    }

    void DeSerialize() { /* 响应不需要反序列化 */ }

    bool ReadContent(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cout << path << " 资源不存在!";
            return false;
        }

        // 定位到文件末尾获取文件大小
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg(); // 获取文件大小
        file.seekg(0, std::ios::beg);

        // 创建足够大的字符串来保存数据
        _resp_body.resize(static_cast<size_t>(fileSize));

        // 读取文件内容到字符串
        file.read(&_resp_body[0], fileSize);
        file.close();

        return true;
    }

    void SetCode(int code)
    {
        if (code >= 100 && code < 600)
        {
            _code = code;
            _desc = CodeToDesc(_code);
        }
        else
        {
            std::cout << "非法的状态码: " << _code << std::endl;
        }
    }

    bool SetHeader(const std::string &key, const std::string &value)
    {
        _resp_headers[key] = value;
        return true;
    }

private:
    std::string _httpversion;
    int _code;
    std::string _desc;
    std::unordered_map<std::string, std::string> _resp_headers;
    std::string _blank_line;
    std::string _resp_body;
};

using func_t = std::function<HttpResponse(HttpRequest &)>;

class Http
{
private:
    std::unordered_map<std::string, func_t> _handlers;

public:
    std::string SuffixToDesc(const std::string &suffix)
    {
        if (suffix == ".html")
            return "text/html";
        else if (suffix == ".css")
            return "text/css";
        else if (suffix == ".js")
            return "application/x-javascript";
        else if (suffix == ".png")
            return "image/png";
        else if (suffix == ".jpg")
            return "image/jpeg";
        else if (suffix == ".txt")
            return "text/plain";
        else
            return "text/html";
    }

    Http() = default;
    ~Http() = default;

    void Register(const std::string &action, func_t handler)
    {
        std::string key = webroot;
        key += action;
        _handlers[key] = handler;
    }

    std::string HandlerRequest(std::string &requeststr)
    {
        std::string respstr;
        HttpRequest req;
        std::cout << requeststr;
        if (req.Deserialize(requeststr))
        {
            HttpResponse resp;
            // 1.交互式处理
            std::string target = req.GetPath();
            if (_handlers.find(target) != _handlers.end())
            {
                resp = _handlers[target](req);
            }
            else
            {   //2. 静态处理
                if (resp.ReadContent(req.GetPath()))
                {
                    std::string suffix = req.Suffix();
                    std::string mime_type_value = SuffixToDesc(suffix);
                    resp.SetHeader("Content-Type", mime_type_value);
                    resp.SetCode(200);
                }
                else
                {
                    resp.SetCode(302);
                    resp.SetHeader("Location","/404.html");
                }
            }
            respstr = resp.Serialize();
        }
        return respstr;
    }
};