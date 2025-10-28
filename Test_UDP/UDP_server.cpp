#include "UDP_server.hpp"
#include <iostream>
#include <memory>

void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " localport" << std::endl;
}
int main(int argc, char* argv[])
{
    if(argc != 2){
        Usage(argv[0]);
        exit(0);
    }

    uint16_t port = std::stoi(argv[1]);

    EnableConsoleSink();
    std::unique_ptr<udp_server> usvr = std::make_unique<udp_server>(port);
    usvr->Init();
    LOG_DEBUG << "server init successfully!";
    usvr->Start();

    return 0;
}