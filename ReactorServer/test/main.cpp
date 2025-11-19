#include "../http/HttpServer.hpp"
#include "login.hpp"
#include "characters.hpp"

using namespace webserver;

#define WWWROOT "../wwwroot/"
void RegisterHandlers(http::HttpServer& server) {
    // 注册路由处理函数
    server.Post("/api/register", _Register);
    server.Post("/api/login", _Login);
    server.Get("/api/characters", HandleCharacterList);
}
int main() 
{
    EnableConsoleSink();
    http::HttpServer server(8080);
    server.SetThreadCount(4);
    server.SetBaseDir(WWWROOT);
    RegisterHandlers(server);
    server.Listen();

    return 0;
}