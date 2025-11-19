#include "../http/HttpServer.hpp"
#include <string>
#include <regex>
#include <unordered_set>
#include <unordered_map>
#include <ctime>

using namespace webserver;

// 模拟内存中的用户存储（实际应用中应该使用数据库）
std::unordered_set<std::string> registeredUsernames;
std::unordered_set<std::string> registeredEmails;

// 模拟用户密码存储（用户名/邮箱 -> 密码映射）
std::unordered_map<std::string, std::string> userCredentials;

// 工具函数
namespace utils {
    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buffer);
    }
    
    bool isValidEmail(const std::string& email) {
        const std::regex pattern(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
        return std::regex_match(email, pattern);
    }
    
    bool isValidUsername(const std::string& username) {
        // 用户名：3-20个字符，只能包含字母、数字、下划线
        const std::regex pattern(R"(^[a-zA-Z0-9_]{3,20}$)");
        return std::regex_match(username, pattern);
    }
    
    bool isValidPassword(const std::string& password) {
        // 密码：至少8位，包含字母和数字
        if (password.length() < 8) return false;
        
        bool has_letter = false;
        bool has_digit = false;
        
        for (char c : password) {
            if (std::isalpha(c)) has_letter = true;
            if (std::isdigit(c)) has_digit = true;
        }
        
        return has_letter && has_digit;
    }
    
    // 简单的JSON构建函数（实际应用中应该使用JSON库）
    std::string buildJsonResponse(bool success, const std::string& message, const std::string& data = "null") {
        return R"({"success": )" + std::string(success ? "true" : "false") + 
               R"(, "message": ")" + message + R"(", "data": )" + data + "}";
    }
    
    // 简单的JSON解析函数（实际应用中应该使用JSON库）
    bool parseJsonField(const std::string& json, const std::string& field, std::string& value) {
        size_t pos = json.find("\"" + field + "\":");
        if (pos == std::string::npos) return false;
        
        pos = json.find("\"", pos + field.length() + 3);
        if (pos == std::string::npos) return false;
        
        size_t end_pos = json.find("\"", pos + 1);
        if (end_pos == std::string::npos) return false;
        
        value = json.substr(pos + 1, end_pos - pos - 1);
        return true;
    }
    
    // 生成简单的用户令牌（实际应用中应该使用JWT等安全机制）
    std::string generateToken(const std::string& username) {
        // 简单的令牌生成，实际应用中应该使用更安全的方法
        std::hash<std::string> hasher;
        return "token_" + std::to_string(hasher(username + utils::getCurrentTime()));
    }
}

void _Register(const http::HttpRequest &req, http::HttpResponse *rsp) {
    // 设置默认响应头
    rsp->SetHeader("Content-Type", "application/json");
    
    // 1. 检查请求方法
    if (req._method != "POST") {
        rsp->_status = 405; // Method Not Allowed
        rsp->_body = utils::buildJsonResponse(false, "方法不允许");
        return;
    }
    
    // 2. 获取请求体
    std::string body = req._body;
    if (body.empty()) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "请求体为空");
        return;
    }
    
    // 3. 解析请求体中的字段
    std::string username, email, password;
    
    if (!utils::parseJsonField(body, "username", username)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "缺少用户名字段");
        return;
    }
    
    if (!utils::parseJsonField(body, "email", email)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "缺少邮箱字段");
        return;
    }
    
    if (!utils::parseJsonField(body, "password", password)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "缺少密码字段");
        return;
    }
    
    // 4. 验证字段格式
    if (!utils::isValidUsername(username)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "用户名必须为3-20个字符，只能包含字母、数字和下划线");
        return;
    }
    
    if (!utils::isValidEmail(email)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "请输入有效的电子邮箱地址");
        return;
    }
    
    if (!utils::isValidPassword(password)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "密码必须至少8位，包含字母和数字");
        return;
    }
    
    // 5. 检查用户是否已存在（使用内存存储模拟）
    if (registeredUsernames.find(username) != registeredUsernames.end()) {
        rsp->_status = 409;
        rsp->_body = utils::buildJsonResponse(false, "用户名已被注册");
        return;
    }
    
    if (registeredEmails.find(email) != registeredEmails.end()) {
        rsp->_status = 409;
        rsp->_body = utils::buildJsonResponse(false, "邮箱已被注册");
        return;
    }
    
    // 6. 注册用户（存储到内存中）
    registeredUsernames.insert(username);
    registeredEmails.insert(email);
    userCredentials[username] = password; // 存储用户名-密码映射
    userCredentials[email] = password;    // 存储邮箱-密码映射
    
    // 7. 记录注册信息（在实际应用中应该存储到数据库）
    std::cout << "用户注册成功 - 用户名: " << username 
              << ", 邮箱: " << email 
              << ", 时间: " << utils::getCurrentTime() << std::endl;
    
    // 8. 返回成功响应
    rsp->_status = 200;
    rsp->_body = utils::buildJsonResponse(true, "注册成功");
}

// 登录接口
void _Login(const http::HttpRequest &req, http::HttpResponse *rsp) {
    // 设置默认响应头
    rsp->SetHeader("Content-Type", "application/json");
    
    // 1. 检查请求方法
    if (req._method != "POST") {
        rsp->_status = 405; // Method Not Allowed
        rsp->_body = utils::buildJsonResponse(false, "方法不允许");
        return;
    }
    
    // 2. 获取请求体
    std::string body = req._body;
    if (body.empty()) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "请求体为空");
        return;
    }
    
    // 3. 解析请求体中的字段
    std::string username, password;
    
    if (!utils::parseJsonField(body, "username", username)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "缺少用户名或邮箱字段");
        return;
    }
    
    if (!utils::parseJsonField(body, "password", password)) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "缺少密码字段");
        return;
    }
    
    // 4. 验证字段格式
    if (username.empty() || password.empty()) {
        rsp->_status = 400;
        rsp->_body = utils::buildJsonResponse(false, "用户名/邮箱和密码不能为空");
        return;
    }
    
    // 5. 检查用户是否存在
    auto it = userCredentials.find(username);
    if (it == userCredentials.end()) {
        rsp->_status = 401; // Unauthorized
        rsp->_body = utils::buildJsonResponse(false, "用户名或密码错误");
        return;
    }
    
    // 6. 验证密码
    if (it->second != password) {
        rsp->_status = 401; // Unauthorized
        rsp->_body = utils::buildJsonResponse(false, "用户名或密码错误");
        return;
    }
    
    // 7. 生成用户令牌（实际应用中应该使用JWT等安全机制）
    std::string token = utils::generateToken(username);
    
    // 8. 构建用户信息（在实际应用中应该从数据库获取）
    std::string userInfo = R"({
        "token": ")" + token + R"(",
        "user": {
            "username": ")" + username + R"(",
            "email": ")" + (registeredEmails.find(username) != registeredEmails.end() ? username : "user@example.com") + R"(",
            "avatar": "https://example.com/avatar.png"
        }
    })";
    
    // 9. 记录登录信息
    std::cout << "用户登录成功 - 用户名: " << username 
              << ", 时间: " << utils::getCurrentTime() << std::endl;
    
    // 10. 返回成功响应
    rsp->_status = 200;
    rsp->_body = utils::buildJsonResponse(true, "登录成功", userInfo);
}

// 路由注册函数（示例）
