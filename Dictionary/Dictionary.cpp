#include "Dictionary.h"

Dictionary::Dictionary(const std::string& path)
    : _path(path)
    {
        LOG_INFO << "create Dictionary obj";
        LoadConf();
    }

void Dictionary::LoadConf()
{
    std::ifstream in(_path);
    if(!in.is_open()){
        LOG_FATAL << "open dict failed";
        return;
    }
    LOG_INFO << "open dict successfully";
    std::string line;
    while(getline(in, line)){
        LOG_DEBUG << "load dict message: " << line;
        auto pos = line.find(sep);
        if(pos == std::string::npos){
            LOG_WARNING << "format error";
            continue;
        }
        std::string word = line.substr(0, pos);
        std::string value = line.substr(pos + sep.size());
        if(word.empty() || value.empty()){
            LOG_WARNING << "format error, word or value empty";
            continue;
        }
        _dict.insert(std::make_pair(word, value));
    }

    in.close();
}

std::string Dictionary::Translate(const std::string& word, const std::string& whoip, uint16_t whoport)
{
    auto iter = _dict.find(word);
    if(iter == _dict.end()){
        return "unkown word";
    }
    return iter->first + "->" + iter->second;
}