#pragma once
#include<functional>
#include<vector>

using namespace std;

class Tool
{
public:
  Tool(){};
  void push_back(const function<void()>& func)
  {
    func_list.push_back(func);
  }
  void execute()
  {
    for(auto& func : func_list)
    {
      func();
    }
  }
  ~Tool(){};
private:
  vector<function<void()>> func_list;
};
