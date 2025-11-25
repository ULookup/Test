#pragma once

#include "../src/Buffer.hpp"
#include "../Logger/Logger.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <sys/stat.h>

namespace webserver::util
{

std::unordered_map<int, std::string> status_message = {
    {100, "Continue"},
    {101, "Switching Protocol"},
    {102, "Processing"},
    {103, "Early Hints"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},
    {300, "Multiple Choice"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {306, "unused"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},          
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}
};

std::unordered_map<std::string, std::string> mime_message = {
    {".aac", "audio/aac"},
    {".abw", "application/x-abiword"},
    {".arc", "application/x-freearc"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bmp", "image/bmp"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".csh", "application/x-csh"},
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformatsofficedocument.wordprocessingml.document"},
    {".eot", "application/vnd.ms-fontobject"},
    {".epub", "application/epub+zip"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".ico", "image/vnd.microsoft.icon"},
    {".ics", "text/calendar"},
    {".jar", "application/java-archive"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".jsonld", "application/ld+json"},
    {".mid", "audio/midi"},
    {".midi", "audio/x-midi"},
    {".mjs", "text/javascript"},
    {".mp3", "audio/mpeg"},
    {".mpeg", "video/mpeg"},
    {".mpkg", "application/vnd.apple.installer+xml"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".oga", "audio/ogg"},
    {".ogv", "video/ogg"},
    {".ogx", "application/ogg"},
    {".otf", "font/otf"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformatsofficedocument.presentationml.presentation"},
    {".rar", "application/x-rar-compressed"},
    {".rtf", "application/rtf"},
    {".sh", "application/x-sh"},
    {".svg", "image/svg+xml"},
    {".swf", "application/x-shockwave-flash"},
    {".tar", "application/x-tar"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".ttf", "font/ttf"},
    {".txt", "text/plain"},
    {".vsd", "application/vnd.visio"},
    {".wav", "audio/wav"},
    {".weba", "audio/webm"},
    {".webm", "video/webm"},
    {".webp", "image/webp"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".xhtml", "application/xhtml+xml"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformatsofficedocument.spreadsheetml.sheet"},
    {".xml", "application/xml"},
    {".xul", "application/vnd.mozilla.xul+xml"},
    {".zip", "application/zip"},
    {".3gp", "video/3gpp"},
    {".3g2", "video/3gpp2"},
    {".7z", "application/x-7z-compressed"}
};

class Util
{
public:
    /* brief: 字符串分割 */
    static size_t Split(const std::string_view &src, const std::string_view &sep, std::vector<std::string_view> *arry) {
        size_t offset = 0;
        while(offset < src.size()) {
            size_t pos = src.find(sep, offset);
            if(pos == std::string::npos) {
                if(pos == src.size()) break;
                arry->push_back(src.substr(offset)); //将剩余部分当作一个字串，放入 arry 中
                return arry->size();
            }
            if(pos == offset) {
                offset = pos + sep.size();
                continue; // 当前字串为空,没有内容
            }
            arry->push_back(src.substr(offset, pos - offset));
            offset = pos + sep.size();
        }

        return arry->size();
    }
    /* brief: 读取文件所有内容，将读取的内容放到 Buffer 里 */
    static bool ReadFile(const std::string &filename, std::string *buf) {
        std::ifstream ifs(filename, std::ios::binary);
        if(ifs.is_open() == false) {
            LOG_ERROR << "open " << filename << " failed!";
            return false;
        }
        size_t fsize = 0;
        ifs.seekg(0, ifs.end);
        fsize =ifs.tellg();
        ifs.seekg(0, ifs.beg); //跳转到起始位置

        buf->resize(fsize);
        ifs.read(&(*buf)[0], fsize);
        if(ifs.good() == false) {
            LOG_ERROR << "read " << filename << " failed!";
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }
    /* brief: 写入文件内容 */
    static bool WriteFile(const std::string &filename, const std::string &buf) {
        std::ofstream ofs(filename, std::ios::binary);
        if(ofs.is_open() == false) {
            LOG_ERROR << "open " << filename << " failed!";
            return false;
        }
        ofs.write(buf.c_str(), buf.size());
        if(ofs.good() == false) {
            LOG_ERROR << "write " << filename << " failed!";
            ofs.close();
            return false;
        }
        ofs.close();
        return true;
    }
    /* brief: URL编码，避免URL中资源路径与查询字符串中的特殊字符与Http请求中特殊字符产生歧义 */
    /* 编码格式：将特殊字符的 ASCII 值，转换成两个16进制字符，前缀% eg: C++ -> C%2B%2B     */
    /* 不编码的特殊字符： RFC3986文档规定 . - _ ~ 以及 字母 和 数字 属于绝对不编码字符      */
    /* RFC3986文档规定，编码格式 %HH                                                     */
    /* W3C标准规定，查询字符串中的空格，需要编码为 + 解码则是 + 转 空格                     */
    static std::string UrlEncode(const std::string &url, bool convert_space_to_plus) {
        std::string res;
        for(auto &c : url) {
            if(c == '.' || c == '-' || c == '_' || c == '~' || isalnum(c) == true) {
                res += c;
                continue;
            }
            if(c == ' ' && convert_space_to_plus) {
                res += '+';
                continue;
            }
            //剩下的需要编码为 %HH 格式
            char tmp[4] = {0};
            snprintf(tmp, 4, "%%%02X", c);
            res += tmp;
        }
        return res;
    }
    static char HexToInt(char c) {
        if(c >= '0' && c <= '9') {
            return c - '0';
        }else if(c >= 'a' && c <= 'z') {
            return c - 'a' + 10;
        }else if(c >= 'A' && c <= 'Z') {
            return c - 'A' + 10;
        }
        return -1;
    }
    /* brief: URL解码 */
    static std::string UrlDecode(const std::string_view &url, bool convert_space_to_plus) {
        //遇到了 % 则将后面两个字符转化为数字，第一位数字左移4位，然后加上第二位数字  eg: + -> 2b  %2b -> 2 << 4 + 11
        std::string res;
        for(int i = 0; i < url.size(); i++) {
            if(url[i] == '+' && convert_space_to_plus == true) {
                res += ' ';
                continue;
            }
            if(url[i] == '%' && (i + 2) < url.size()) {
                char v1 = HexToInt(url[i + 1]);
                char v2 = HexToInt(url[i + 2]);
                char v = (v1 <<  4) + v2;
                res += v;
                i += 2;
                continue;
            }
            res += url[i];
        }
        return res;
    }
    /* brief: 响应状态码的描述信息获取 */
    static std::string StatusDesc(int status) {
        auto it = status_message.find(status);
        if(it != status_message.end()) {
            return it->second;
        }
        return "Unknow";
    }
    /* brief: 根据文件后缀名获取文件mime */
    static std::string ExtMime(const std::string &filename) {
        size_t pos = filename.find_last_of('.');
        if(pos == std::string::npos) {
            return "application/octet-stream";
        }
        std::string ext = filename.substr(pos);
        auto it = mime_message.find(ext);
        if(it == mime_message.end()) {
            return "application/octet-stream";
        }
        return it->second;
    }
    /* brief: 判断文件是否是一个目录 */
    static bool IsDirectory(const std::string &filename) {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if(ret < 0) return false;
        return S_ISDIR(st.st_mode);
    }
    /* brief: 判断文件是否是一个普通文件 */
    static bool IsRegular(const std::string &filename) {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if(ret < 0) return false;
        return S_ISREG(st.st_mode);
    }
    /* brief: http请求的资源路径有效性判断 */
    static bool ValidPath(const std::string &path) {
        //思想：按照 / 进行路径分割，根据有多少子目录，有多少层，深度不能小于0
        std::vector<std::string_view> subdir;
        Split(path, "/", &subdir);
        int level = 0;
        for(auto &dir : subdir) {
            if(dir == "..") {
                level--;
                if(level < 0) return false;
                continue;
            }
            level++;
        }
        return true;
    }
    /* brief: 移除字符串尾部的 \r\n 或 \n */
    static void TrimCrlf(std::string &line) {
        if (line.empty()) return;

        // 检查并移除 \n
        if (line.back() == '\n') {
            line.pop_back();
            if (line.empty()) return; // 只有 \n
        }

        // 检查并移除 \r
        if (line.back() == '\r') {
            line.pop_back();
        }
    }
    
    static std::vector<std::string_view> SplitLine(std::string_view line) {
        std::vector<std::string_view> result;
        size_t current_global_pos = 0; // 用于在 line 上跟踪 Method 结束的位置

        // 1. 查找第一个空格，分隔 Method
        size_t method_end = line.find(' ');
        
        // 处理只有方法或格式错误的情况
        if (method_end == std::string_view::npos) {
            // 如果找不到空格，则将整行视为方法（或错误情况）
            result.emplace_back(line);
            // 为了确保返回固定数量的元素，可以填充空值，但通常仅返回方法即可
            // 实际应用中，这里可能抛出异常或返回错误码
            result.emplace_back(""); // 路径
            result.emplace_back(""); // 参数
            result.emplace_back(""); // 版本
            return result; 
        }
        
        // 放入 Method
        result.emplace_back(line.substr(0, method_end));
        current_global_pos = method_end + 1; // URI/Version 的开始位置

        // 2. 查找第二个空格，分隔 URI 和 Version
        size_t uri_end = line.find(' ', current_global_pos);
        
        std::string_view uri;
        if (uri_end == std::string_view::npos) {
            // 缺少 HTTP Version
            uri = line.substr(current_global_pos);
            // 在实际应用中，这里可能抛出异常或使用默认版本
        } else {
            // URI 部分 (不含 Version)
            uri = line.substr(current_global_pos, uri_end - current_global_pos);
        }

        // 3. 在 URI 内部查找 '?'，分隔 Path 和 Params
        size_t path_sep = uri.find('?'); // 从 uri 的局部索引 0 开始查找
        
        if (path_sep == std::string_view::npos) {
            // 没有参数
            result.emplace_back(uri);  // Path
            result.emplace_back("");   // Query Params (空)
        } else {
            // 放入 Path (局部索引 0 到 path_sep)
            result.emplace_back(uri.substr(0, path_sep));
            // 放入 Params (局部索引 path_sep+1 到结尾)
            result.emplace_back(uri.substr(path_sep + 1));
        }
        
        // 4. 放入 Version
        if (uri_end != std::string_view::npos) {
            result.emplace_back(line.substr(uri_end + 1)); // Version
        } else {
            result.emplace_back(""); // Version 缺失
        }

        return result;
    } 
};

}