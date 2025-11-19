#include "../Logger/Logger.h"
#include "../http/HttpServer.hpp"
#include <fstream>

using namespace webserver;

bool ReadFileToBuffer(src::Buffer &buf, const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    buf.Clear();
    char tmp[4096];
    while (file.good()) {
        file.read(tmp, sizeof(tmp));
        std::streamsize n = file.gcount();
        if (n > 0) {
            buf.WriteAndPush(tmp, (uint64_t)n);
        }
    }
    return true;
}


void HandleCharacterList(const http::HttpRequest& req, http::HttpResponse *res)
{
    static src::Buffer cached;
    static bool loaded = false;

    // 首次读取 JSON 文件
    if (!loaded) {
        if (!ReadFileToBuffer(cached, "data/characters.json")) {
            res->_status = 500;
            res->SetHeader("Content-Type", "application/json; charset=utf-8");
            res->_body = R"({"error":"failed to load character data"})";
            return;
        }
        loaded = true;
    }

    res->_status = 200;
    res->SetHeader("Content-Type", "application/json; charset=utf-8");

    // 将 Buffer 转为 std::string
    std::string body = cached.ReadAsString(cached.ReadAbleBytes());
    res->_body = std::move(body);
}

