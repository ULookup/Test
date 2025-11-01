#include "Http.hpp"
#include "TcpServer.h"

void Usage(std::string proc)
{
    std::cout << "Usage: " << proc << " localport" << std::endl;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        Usage(argv[0]);
        exit(0);
    }
    std::unique_ptr<Http> http = std::make_unique<Http>();

    uint16_t serverport = std::stoi(argv[1]);
    std::unique_ptr<Server> server = std::make_unique<Server>(serverport, [&http](std::string& reqstr)->std::string{ return http->HandlerRequest(reqstr); });
    server->Run();
    return 0;
}