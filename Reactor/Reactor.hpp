#pragma once

#include "Connection.hpp"
#include <unordered_map>
#include <memory>

class Reactor
{
private:
public:
    std::unordered_map<int, std::shared_ptr<Connection>> _connections;
};