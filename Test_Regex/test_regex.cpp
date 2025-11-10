#include <iostream>
#include <string>
#include <regex>

int main()
{
    std::string str = "GET /home/login?user=icepop&pass=114514 HTTP/1.1\r\n";
    std::smatch matches;
    std::regex e("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?");
    //（XXX）表示匹配并提取括号内的字符串
    //[^?] 表示匹配非问号字符，*表示0次或多次
    //X(.*) 表示匹配X之后的任意字符0次或多次，直到遇到空格
    //(?:XXXXX) 表示匹配后面的某个格式的字符串，但是不提取
    //(XXX)? 这个问号表示匹配前面的表达式0次或1次
    std::regex_match(str, matches, e);
    for(auto& e: matches){
        std::cout << e << std::endl;
    }
    
    return 0;
}