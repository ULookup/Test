#pragma once

#include "Logger.h"
#include <iostream>
#include <unordered_map>
#include <string>

static const std::string sep = ": ";

class Dictionary
{
private:
    void LoadConf();
public:
    Dictionary(const std::string& path);

    std::string Translate(const std::string& word, const std::string& whoip, uint16_t whoport);

    ~Dictionary() = default;
private:
    std::string _path;
    std::unordered_map<std::string,std::string> _dict;
};