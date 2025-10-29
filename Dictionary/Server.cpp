#include "Dictionary.h"
#include "UdpServer.h"

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
    EnableConsoleSink();

    uint16_t port = std::stoi(argv[1]);
    Dictionary dict("./dict.txt");
    std::unique_ptr<DictServer> usvr = std::make_unique<DictServer>(port, 
        [&dict](const std::string& word, const std::string& whoip, uint16_t whoport)->std::string{
            return dict.Translate(word, whoip, whoport); });
    
    usvr->Init();
    LOG_DEBUG << "dict Init() successfully";
    usvr->Start(); 
    usvr->Stop();
    return 0;
}