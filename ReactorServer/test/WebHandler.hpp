#pragma once

#include "../Logger/Logger.h"
#include "../include/utils/NotificationService.hpp"
#include "../include/utils/MultipartParser.h"
#include "../include/storage/JsonStorage.h"
#include "../include/utils/TokenUtil.hpp"
#include "../include/utils/Auth.hpp"
#include "../http/HttpServer.hpp"
#include "storageInstance.hpp"
#include <openssl/sha.h>
#include <ctime>
#include <random>
#include <fstream>

using namespace webserver;

bool containsIgnoreCase(const std::string &s, const std::string &key) {
    if (key.empty()) return false;

    std::string a = s, b = key;
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    std::transform(b.begin(), b.end(), b.begin(), ::tolower);

    return a.find(b) != std::string::npos;
}

std::string Sha256(const std::string &s)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)s.c_str(), s.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}
std::string GenerateToken(int uid)
{
    std::stringstream ss;
    ss << uid << "_" << time(NULL) << "_" << rand();
    return Sha256(ss.str());
}

void RegisterHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[RegisterHandler] 进入函数";
    json body;
    try {
        body = json::parse(req._body);
    } catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid json"}});
        return;
    }

    // 提取字段
    std::string username = body.value("username", "");
    std::string email = body.value("email", "");
    std::string password = body.value("password", "");

    // 基础校验
    if (username.empty() || email.empty() || password.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 加载用户列表
    std::vector<User> users;
    storage->LoadUsers(users);

    // 重复校验
    for (auto &u : users) {
        if (u.username == username) {
            rsp->Json({{"code", 2001}, {"msg", "username exists"}});
            return;
        }
        if (u.email == email) {
            rsp->Json({{"code", 2001}, {"msg", "email exists"}});
            return;
        }
    }

    // 生成新 ID
    int newId = users.empty() ? 1 : (users.back().id + 1);

    // 简单加密（你可换成 bcrypt）
    std::string pwd_hash = Sha256(password);

    // 创建用户对象
    User newUser;
    newUser.id = newId;
    newUser.username = username;
    newUser.email = email;
    newUser.password_hash = pwd_hash;
    newUser.avatar = "/static/avatar/default.png";
    newUser.bio = "";
    newUser.create_time = time(NULL);

    users.push_back(newUser);

    // 保存
    if (!storage->SaveUsers(users)) {
        rsp->Json({{"code", 5001}, {"msg", "server_error"}});
        return;
    }

    // 响应
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"id", newId},
            {"username", username},
            {"email", email}
        }}
    });
}

void LoginHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[LoginHandler] 进入函数";

    json body;
    try {
        body = json::parse(req._body);
    } catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid json"}});
        return;
    }

    std::string account = body.value("account", ""); // username or email
    std::string password = body.value("password", "");

    if (account.empty() || password.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 1. Load users
    std::vector<User> users;
    storage->LoadUsers(users);

    User* found = nullptr;
    for (auto& u : users) {
        if (u.username == account || u.email == account) {
            found = &u;
            break;
        }
    }

    if (!found) {
        rsp->Json({{"code", 2001}, {"msg", "user_not_found"}});
        return;
    }

    // 2. Verify password
    std::string hash = Sha256(password);
    if (hash != found->password_hash) {
        rsp->Json({{"code", 2002}, {"msg", "wrong_password"}});
        return;
    }

    // 3. Generate token
    std::string token = GenerateToken(found->id);
    LOG_DEBUG << "[LoginHandler] 新 token = " << token;

    // 4. 保存 token（内存）
    SaveToken(token, found->id);

    // 5. 写入 users.json（持久化）
    found->token = token;   // ← 你只需要 User.h 添加一个 string token 字段即可
    storage->SaveUsers(users);

    LOG_DEBUG << "[LoginHandler] token 持久化写入成功";

    // 6. 返回
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"token", token},
            {"user", {
                {"id", found->id},
                {"username", found->username},
                {"email", found->email},
                {"avatar", found->avatar},
                {"bio", found->bio}
            }}
        }}
    });

    LOG_DEBUG << "[LoginHandler] 登录成功，响应已返回";
}


void UserInfoHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserInfoHandler] 进入函数";
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    std::vector<User> users;
    storage->LoadUsers(users);

    for (auto &u : users) {
        if (u.id == uid) {
            rsp->Json({
                {"code", 0},
                {"msg", "success"},
                {"data", {
                    {"id", u.id},
                    {"username", u.username},
                    {"email", u.email},
                    {"avatar", u.avatar},
                    {"bio", u.bio},
                    {"post_count", u.post_count},
                    {"like_count", u.like_count},
                    {"follower_count", u.follower_count},
                    {"following_count", u.following_count},
                    {"create_time", u.create_time}
                }}
            });
            return;
        }
    }

    rsp->Json({{"code", 2001}, {"msg", "user_not_found"}});
}

void UserUpdateHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserUpdateHandler] 进入函数";
    // 1. token 鉴权
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. parse body
    json body;
    try {
        body = json::parse(req._body);
    } catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid json"}});
        return;
    }

    std::string newUsername = body.value("username", "");
    std::string newBio = body.value("bio", "");

    // 3. load user list
    std::vector<User> users;
    storage->LoadUsers(users);

    User* target = nullptr;
    for (auto &u : users) {
        if (u.id == uid) {
            target = &u;
            break;
        }
    }

    if (!target) {
        rsp->Json({{"code", 2001}, {"msg", "user_not_found"}});
        return;
    }

    // 4. username 重名检查
    if (!newUsername.empty() && newUsername != target->username)
    {
        for (auto &u : users) {
            if (u.username == newUsername) {
                rsp->Json({{"code", 2001}, {"msg", "username exists"}});
                return;
            }
        }
        target->username = newUsername;
    }

    // 5. 更新 bio
    if (!newBio.empty()) {
        target->bio = newBio;
    }

    // 6. 保存到 JSON
    if (!storage->SaveUsers(users)) {
        rsp->Json({{"code", 5001}, {"msg", "server_error"}});
        return;
    }

    // 7. 返回成功
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"id", target->id},
            {"username", target->username},
            {"email", target->email},
            {"avatar", target->avatar},
            {"bio", target->bio},
            {"post_count", target->post_count},
            {"like_count", target->like_count},
            {"follower_count", target->follower_count},
            {"following_count", target->following_count}
        }}
    });
}

void PostListHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[PostListHandler] 进入函数";
    // 1. 获取分页参数
    int page = 1;
    int size = 10;

    try {
        page = std::stoi(req.GetParam("page"));
        size = std::stoi(req.GetParam("size"));
    } catch (...) {}

    if (page <= 0) page = 1;
    if (size <= 0) size = 10;

    // 2. 加载所有帖子
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    // 3. 按时间倒序
    std::sort(posts.begin(), posts.end(), [](const Post &a, const Post &b) {
        return a.create_time > b.create_time;
    });

    int total = posts.size();

    // 4. 计算分页区间
    int start = (page - 1) * size;
    int end = std::min(start + size, total);

    if (start >= total) {
        // 请求页超出范围，返回空
        rsp->Json({
            {"code", 0},
            {"msg", "success"},
            {"data", {
                {"list", json::array()},
                {"page", page},
                {"size", size},
                {"total", total}
            }}
        });
        return;
    }

    // 5. 构造返回列表
    json list = json::array();
    for (int i = start; i < end; i++) {
        const Post &p = posts[i];
        list.push_back({
            {"id", p.id},
            {"author_id", p.author_id},
            {"title", p.title},
            {"content", p.content},
            {"images", p.images},
            {"tags", p.tags},
            {"create_time", p.create_time},
            {"like_count", p.like_count},
            {"comment_count", p.comment_count},
            {"fav_count", p.fav_count}
        });
    }

    // 6. 返回 JSON
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"list", list},
            {"page", page},
            {"size", size},
            {"total", total}
        }}
    });
}

void PostDetailHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[PostDetailHandler] 进入函数";
    // 1. 获取 URL 中的 {id}
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 2. 加载本地存储的所有帖子
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    // 3. 查找目标帖子
    const Post* target = nullptr;
    for (auto &p : posts) {
        if (p.id == postId) {
            target = &p;
            break;
        }
    }

    if (!target) {
        rsp->Json({{"code", 3001}, {"msg", "post_not_found"}});
        return;
    }

    // 4. 返回帖子详细数据
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"id", target->id},
            {"author_id", target->author_id},
            {"title", target->title},
            {"content", target->content},
            {"images", target->images},
            {"tags", target->tags},
            {"create_time", target->create_time},
            {"like_count", target->like_count},
            {"comment_count", target->comment_count},
            {"fav_count", target->fav_count}
        }}
    });
}

void CommentListHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[CommentListHandler] 进入函数";

    // 1. 获取帖子ID
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch(...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 2. 加载评论
    std::vector<Comment> comments;
    storage->LoadComments(comments);

    // 加载用户数据（用于补充用户名和头像）
    std::vector<User> users;
    storage->LoadUsers(users);

    auto findUser = [&](int uid){
        for (auto &u : users) if (u.id == uid) return u;
        return User{};
    };

    json list = json::array();

    // 3. 组装评论 + 用户信息
    for (auto &c : comments) {
        if (c.post_id == postId) {

            User u = findUser(c.user_id);

            list.push_back({
                {"id", c.id},
                {"post_id", c.post_id},
                {"user_id", c.user_id},
                {"username", u.username},
                {"avatar", u.avatar},
                {"content", c.content},
                {"create_time", c.create_time}
            });
        }
    }

    // 4. 排序
    std::sort(list.begin(), list.end(),
        [](const json &a, const json &b){
            return a["create_time"].get<long long>() < b["create_time"].get<long long>();
        }
    );

    // 5. 返回
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}


void CommentCreateHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[CommentCreateHandler] 进入函数";
    // 1. 登录校验
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 获取帖子ID
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch(...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 3. 解析 body
    json body;
    try {
        body = json::parse(req._body);
    } catch(...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid json"}});
        return;
    }

    std::string content = body.value("content", "");
    if (content.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "content empty"}});
        return;
    }

    // 4. 加载评论列表
    std::vector<Comment> comments;
    storage->LoadComments(comments);

    // 5. 为评论分配 ID
    int newId = comments.empty() ? 1 : (comments.back().id + 1);

    // 6. 构造 Comment 对象
    Comment comment;
    comment.id = newId;
    comment.post_id = postId;
    comment.user_id = uid;
    comment.content = content;
    comment.create_time = time(NULL);

    comments.push_back(comment);

    // 7. 保存评论列表
    if (!storage->SaveComments(comments)) {
        rsp->Json({{"code", 5001}, {"msg", "server_error"}});
        return;
    }

    // 8. 加载帖子 + 找出目标帖子对象
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    Post* targetPost = nullptr;
    for (auto &p : posts) {
        if (p.id == postId) {
            targetPost = &p;
            break;
        }
    }

    if (!targetPost) {
        rsp->Json({{"code", 3001}, {"msg", "post_not_found"}});
        return;
    }

    // 评论计数更新
    targetPost->comment_count++;
    storage->SavePosts(posts);

    // 9. 发送通知（不要给自己发）
    if (targetPost->author_id != uid) {
        AddNotification(targetPost->author_id, uid, "comment", postId, content);
    }

    // 10. 返回评论内容
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"id", comment.id},
            {"post_id", comment.post_id},
            {"user_id", comment.user_id},
            {"content", comment.content},
            {"create_time", comment.create_time}
        }}
    });
}


void LikePostHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[LikePostHandler] 进入函数";
    // 1. 登录校验
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 获取帖子 ID
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 3. 加载帖子列表
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    Post* target = nullptr;
    for (auto &p : posts) {
        if (p.id == postId) {
            target = &p;
            break;
        }
    }
    if (!target) {
        rsp->Json({{"code", 3001}, {"msg", "post_not_found"}});
        return;
    }

    // 4. 加载点赞记录
    std::vector<Like> likes;
    storage->LoadLikes(likes);

    // 5. 检查是否已点赞
    for (auto &l : likes) {
        if (l.user_id == uid && l.post_id == postId) {
            rsp->Json({{"code", 0}, {"msg", "already_liked"}});
            return;
        }
    }

    // 6. 添加点赞记录
    Like like;
    like.user_id = uid;
    like.post_id = postId;
    like.time = time(NULL);

    likes.push_back(like);
    storage->SaveLikes(likes);

    // 7. +1 点赞数
    target->like_count++;
    storage->SavePosts(posts);
    if (target->author_id != uid) {
        AddNotification(target->author_id, uid, "like", postId);
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"post_id", postId},
            {"user_id", uid},
            {"like_count", target->like_count}
        }}
    });
}

void UnlikePostHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UnlikePostHandler] 进入函数";
    // 1. 登录校验
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 帖子 ID
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 3. 加载帖子
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    Post* target = nullptr;
    for (auto &p : posts) {
        if (p.id == postId) {
            target = &p;
            break;
        }
    }
    if (!target) {
        rsp->Json({{"code", 3001}, {"msg", "post_not_found"}});
        return;
    }

    // 4. 加载点赞记录
    std::vector<Like> likes;
    storage->LoadLikes(likes);

    // 5. 找到这条点赞记录
    bool found = false;
    for (auto it = likes.begin(); it != likes.end(); ++it) {
        if (it->user_id == uid && it->post_id == postId) {
            likes.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        rsp->Json({{"code", 0}, {"msg", "not_liked"}});
        return;
    }

    // 6. 保存新的点赞列表
    storage->SaveLikes(likes);

    // 7. 点赞数 -1
    if (target->like_count > 0)
        target->like_count--;

    storage->SavePosts(posts);

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"post_id", postId},
            {"user_id", uid},
            {"like_count", target->like_count}
        }}
    });
}

void FavPostHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[FavPostHandler] 进入函数";
    // 1. 登录验证
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 获取帖子 ID
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch(...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 3. 加载帖子
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    Post* target = nullptr;
    for (auto &p : posts) {
        if (p.id == postId) {
            target = &p;
            break;
        }
    }
    if (!target) {
        rsp->Json({{"code", 3001}, {"msg", "post_not_found"}});
        return;
    }

    // 4. 加载收藏列表
    std::vector<Fav> favs;
    storage->LoadFavs(favs);

    // 5. 判断是否已经收藏
    for (auto &f : favs) {
        if (f.user_id == uid && f.post_id == postId) {
            rsp->Json({
                {"code", 0},
                {"msg", "already_fav"},
                {"data", {{"fav_count", target->fav_count}}}
            });
            return;
        }
    }

    // 6. 添加收藏记录
    Fav fav;
    fav.user_id = uid;
    fav.post_id = postId;
    fav.time = time(NULL);

    favs.push_back(fav);
    storage->SaveFavs(favs);

    // 7. 该帖子收藏数 +1
    target->fav_count++;
    storage->SavePosts(posts);
    if (target->author_id != uid) {
        AddNotification(target->author_id, uid, "fav", postId);
    }


    // 8. 返回成功
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"post_id", postId},
            {"fav_count", target->fav_count}
        }}
    });
}

void UnfavPostHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UnfavPostHandler] 进入函数";
    // 1. 登录验证
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 获取帖子 ID
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int postId = 0;
    try { postId = std::stoi(sid); }
    catch(...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    // 3. 加载帖子
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    Post* target = nullptr;
    for (auto &p : posts) {
        if (p.id == postId) {
            target = &p;
            break;
        }
    }
    if (!target) {
        rsp->Json({{"code", 3001}, {"msg", "post_not_found"}});
        return;
    }

    // 4. 加载收藏记录
    std::vector<Fav> favs;
    storage->LoadFavs(favs);

    bool found = false;

    // 5. 查找并删除该收藏记录
    for (auto it = favs.begin(); it != favs.end(); ++it) {
        if (it->user_id == uid && it->post_id == postId) {
            favs.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        rsp->Json({{"code", 0}, {"msg", "not_fav"}});
        return;
    }

    // 6. 保存收藏列表
    storage->SaveFavs(favs);

    // 7. 收藏数 -1
    if (target->fav_count > 0)
        target->fav_count--;

    storage->SavePosts(posts);

    // 8. 返回成功
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"post_id", postId},
            {"fav_count", target->fav_count}
        }}
    });
}

void UserProfileHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserProfileHandler] 进入函数";
    // sid 先从路径参数（matches）获取
    std::string sid;
    if (req._matches.size() > 1) {
        sid = req._matches[1].str();
    } else {
        sid = req.GetParam("id"); // 兼容 Query 参数
    }

    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int uid = 0;
    try { uid = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    std::vector<User> users;
    storage->LoadUsers(users);

    for (auto &u : users) {
        if (u.id == uid) {
            rsp->Json({
                {"code", 0},
                {"msg", "success"},
                {"data", {
                    {"id", u.id},
                    {"username", u.username},
                    {"email", u.email},
                    {"avatar", u.avatar},
                    {"bio", u.bio},
                    {"post_count", u.post_count},
                    {"like_count", u.like_count},
                    {"follower_count", u.follower_count},
                    {"following_count", u.following_count},
                    {"create_time", u.create_time}
                }}
            });
            return;
        }
    }

    rsp->Json({{"code", 2001}, {"msg", "user_not_found"}});
}

void UserPostsHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserPostsHandler] 进入函数";
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int uid = 0;
    try { uid = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    std::vector<Post> posts;
    storage->LoadPosts(posts);

    json list = json::array();

    for (auto &p : posts) {
        if (p.author_id == uid) {
            list.push_back({
                {"id", p.id},
                {"title", p.title},
                {"content", p.content},
                {"images", p.images},
                {"tags", p.tags},
                {"create_time", p.create_time},
                {"like_count", p.like_count},
                {"comment_count", p.comment_count},
                {"fav_count", p.fav_count}
            });
        }
    }

    // 按时间倒序
    std::sort(list.begin(), list.end(), [](const json &a, const json &b) {
        return a["create_time"].get<long long>() > b["create_time"].get<long long>();
    });

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}

void UserLikesHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserLikesHandler] 进入函数";

    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int uid = 0;
    try { uid = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    std::vector<Like> likes;
    storage->LoadLikes(likes);

    std::vector<Post> posts;
    storage->LoadPosts(posts);

    json list = json::array();

    for (auto &l : likes) {
        if (l.user_id == uid) {
            // 找到该帖子
            for (auto &p : posts) {
                if (p.id == l.post_id) {
                    list.push_back({
                        {"id", p.id},
                        {"title", p.title},
                        {"content", p.content},
                        {"images", p.images},
                        {"tags", p.tags},
                        {"create_time", p.create_time},
                        {"like_count", p.like_count},
                        {"comment_count", p.comment_count},
                        {"fav_count", p.fav_count}
                    });
                    break;
                }
            }
        }
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}

void UserFavsHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserFavsHandler] 进入函数";
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    int uid = 0;
    try { uid = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid_param"}});
        return;
    }

    std::vector<Fav> favs;
    storage->LoadFavs(favs);

    std::vector<Post> posts;
    storage->LoadPosts(posts);

    json list = json::array();

    for (auto &f : favs) {
        if (f.user_id == uid) {
            for (auto &p : posts) {
                if (p.id == f.post_id) {
                    list.push_back({
                        {"id", p.id},
                        {"title", p.title},
                        {"content", p.content},
                        {"images", p.images},
                        {"tags", p.tags},
                        {"create_time", p.create_time},
                        {"like_count", p.like_count},
                        {"comment_count", p.comment_count},
                        {"fav_count", p.fav_count}
                    });
                    break;
                }
            }
        }
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}

void AvatarUploadHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[AvatarUploadHandler] 进入函数";
    // 1. 必须登录
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }
    LOG_DEBUG << "[AvatarUploadHandler] 检查登录成功";

    // 2. 获取 boundary
    std::string contentType = req.GetHeader("Content-Type");
    if (contentType.find("multipart/form-data") == std::string::npos) {
        rsp->Json({{"code", 1001}, {"msg", "invalid content type"}});
        return;
    }

    size_t bpos = contentType.find("boundary=");
    if (bpos == std::string::npos) {
        rsp->Json({{"code", 1001}, {"msg", "no boundary"}});
        return;
    }
    std::string boundary = contentType.substr(bpos + 9);
    LOG_DEBUG << "[AvatarUploadHandler] 获取boundary成功";

    // 3. 解析 multipart
    MultipartParser parser(req._body, boundary);
    if (!parser.Parse()) {
        rsp->Json({{"code", 1001}, {"msg", "multipart parse error"}});
        return;
    }

    if (!parser.HasFile("avatar")) {
        rsp->Json({{"code", 1001}, {"msg", "avatar not found"}});
        return;
    }

    MultipartFile file = parser.GetFile("avatar");
    LOG_DEBUG << "[AvatarUploadHandler] 解析multipart成功";

    // 4. 文件类型检查
     std::string type = file.contentType;

    // 去掉前后空格和 CR/LF
    while (!type.empty() && (type.back() == '\r' || type.back() == '\n' || type.back() == ' '))
        type.pop_back();
    while (!type.empty() && type.front() == ' ')
        type.erase(type.begin());

    LOG_DEBUG << "[AvatarUploadHandler] 检测到文件类型: [" << type << "]";

    if (type != "image/png" &&
        type != "image/jpeg" &&
        type != "image/jpg") {

        rsp->Json({{"code", 1001}, {"msg", "unsupported file type"}});
        return;
    }
    LOG_DEBUG << "[AvatarUploadHandler] 检查文件类型成功";

    // 5. 保存文件
    std::string savePath = "../wwwroot/static/avatar/" + std::to_string(uid) + ".png";
    FileUtil::WriteFileAtomic(savePath, file.data);
    LOG_DEBUG << "[AvatarUploadHandler] 保存文件成功";


    // 6. 更新 users.json
    std::vector<User> users;
    storage->LoadUsers(users);

    for (auto &u : users) {
        if (u.id == uid) {
            u.avatar = "/static/avatar/" + std::to_string(uid) + ".png";
            break;
        }
    }

    storage->SaveUsers(users);
    LOG_DEBUG << "[AvatarUploadHandler] 更新json成功";

    // 7. 返回成功
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"avatar", "/static/avatar/" + std::to_string(uid) + ".png"}
        }}
    });
    LOG_DEBUG << "[AvatarUploadHandler] rsp写入json成功";

}

void HotTopicsHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[HotTopicsHandler] 进入函数";
    std::vector<Topic> topics;
    storage->LoadTopics(topics);

    // 按热度降序
    std::sort(topics.begin(), topics.end(),
              [](const Topic &a, const Topic &b){
                  return a.heat > b.heat;
              });

    json list = json::array();
    for (auto &t : topics) {
        list.push_back({
            {"id", t.id},
            {"name", t.name},
            {"heat", t.heat}
        });
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}

void RecommendPostsHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[RecommendPostsHandler] 进入函数";

    // ===============================
    // 1. 分页参数
    // ===============================
    int page = 1;
    int pageSize = 10;

    try {
        if (!req.GetParam("page").empty())
            page = std::stoi(req.GetParam("page"));
        if (!req.GetParam("page_size").empty())
            pageSize = std::stoi(req.GetParam("page_size"));
    } catch (...) {
        page = 1;
        pageSize = 10;
    }

    if (page <= 0) page = 1;
    if (pageSize <= 0 || pageSize > 50) pageSize = 10;


    // ===============================
    // 2. 加载帖子
    // ===============================
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    std::sort(posts.begin(), posts.end(),
        [](const Post &a, const Post &b){
            int scoreA = a.like_count * 3 + a.comment_count * 2 + a.fav_count * 4;
            int scoreB = b.like_count * 3 + b.comment_count * 2 + b.fav_count * 4;
            return scoreA > scoreB;
        });


    // ===============================
    // 3. 分页截取
    // ===============================
    int start = (page - 1) * pageSize;
    int end   = std::min((int)posts.size(), start + pageSize);

    if (start >= (int)posts.size()) {
        rsp->Json({
            {"code", 0},
            {"msg", "success"},
            {"data", json::array()}
        });
        return;
    }


    // ===============================
    // 4. 加载用户（用于作者信息）
    // ===============================
    std::vector<User> users;
    storage->LoadUsers(users);

    std::unordered_map<int, User> usermap;
    for (auto &u : users) {
        usermap[u.id] = u;
    }


    // ===============================
    // 5. 生成返回 JSON（加入作者信息）
    // ===============================
    json list = json::array();

    for (int i = start; i < end; i++) {
        auto &p = posts[i];

        std::string author_name = "未知用户";
        std::string avatar_path = "/static/avatar/default.png";

        if (usermap.count(p.author_id)) {
            auto &u = usermap[p.author_id];

            // --- 作者名称 ---
            if (!u.username.empty())
                author_name = u.username;

            // --- 作者头像字段兼容处理（avatar / avator）---

            // --- 自动补全路径 ---
            if (!u.avatar.empty()) {
                if (u.avatar.rfind("/static/", 0) == 0)
                    avatar_path = u.avatar; // 已经是完整路径
                else
                    avatar_path = "/static/avatar/" + u.avatar; // 只有文件名
            }
        }

        list.push_back({
            {"id", p.id},
            {"author_id", p.author_id},
            {"author_name", author_name},
            {"author_avatar", avatar_path},   // ← 统一保证正确
            {"title", p.title},
            {"content", p.content},
            {"images", p.images},
            {"tags", p.tags},
            {"create_time", p.create_time},
            {"like_count", p.like_count},
            {"comment_count", p.comment_count},
            {"fav_count", p.fav_count}
        });
    }

    // ===============================
    // 6. 返回
    // ===============================
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}




void RecommendCreatorsHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[RecommendCreatorsHandler] 进入函数";

    std::vector<User> users;
    storage->LoadUsers(users);

    // 若无用户直接返回空
    if (users.empty()) {
        rsp->Json({
            {"code", 0},
            {"msg", "success"},
            {"data", json::array()}
        });
        return;
    }

    // 用户活跃度评分（你原来的逻辑）
    std::sort(users.begin(), users.end(),
        [](const User &a, const User &b){
            int scoreA = a.post_count * 3 + a.like_count;
            int scoreB = b.post_count * 3 + b.like_count;
            return scoreA > scoreB;
        });

    int limit = std::min((int)users.size(), 10);
    json list = json::array();

    for (int i = 0; i < limit; i++) {
        auto &u = users[i];

        // ================================
        // 修复头像路径（核心逻辑）
        // ================================
        std::string avatar = u.avatar;

        if (avatar.empty()) {
            avatar = "/static/avatar/default.png";   // 防御空头像
        } else if (avatar[0] != '/') {
            avatar = "/" + avatar;
        }

        // ================================
        // 生成返回 JSON（新增 followers）
        // ================================
        list.push_back({
            {"id", u.id},
            {"username", u.username},
            {"avatar", avatar},

            // 新增 —— 避免前端 undefined
            {"followers", u.follower_count},

            // 你已有的扩展字段，继续保留
            {"bio", u.bio},
            {"post_count", u.post_count},
            {"like_count", u.like_count}
        });
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}


void SearchPostsHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[SearchPostsHandler] 进入函数";
    std::string q = req.GetParam("q");
    if (q.empty()) {
        rsp->Json({
            {"code", 0},
            {"msg", "success"},
            {"data", json::array()}
        });
        return;
    }

    std::vector<Post> posts;
    storage->LoadPosts(posts);

    json list = json::array();

    for (auto &p : posts) {
        bool match = false;

        // 标题匹配
        if (containsIgnoreCase(p.title, q)) match = true;

        // 内容匹配（截断减少性能开销）
        if (!match) {
            std::string shortContent = p.content.substr(0, 200);
            if (containsIgnoreCase(shortContent, q)) match = true;
        }

        // 标签匹配
        if (!match) {
            for (auto &tag : p.tags) {
                if (containsIgnoreCase(tag, q)) {
                    match = true;
                    break;
                }
            }
        }

        if (!match) continue;

        list.push_back({
            {"id", p.id},
            {"author_id", p.author_id},
            {"title", p.title},
            {"content", p.content},
            {"images", p.images},
            {"tags", p.tags},
            {"create_time", p.create_time},
            {"like_count", p.like_count},
            {"comment_count", p.comment_count},
            {"fav_count", p.fav_count}
        });
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}

void SearchUsersHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[SearchUsersHandler] 进入函数";
    std::string q = req.GetParam("q");
    if (q.empty()) {
        rsp->Json({
            {"code", 0},
            {"msg", "success"},
            {"data", json::array()}
        });
        return;
    }

    std::vector<User> users;
    storage->LoadUsers(users);

    json list = json::array();

    for (auto &u : users) {
        bool match = false;

        if (containsIgnoreCase(u.username, q)) match = true;
        if (!match && containsIgnoreCase(u.bio, q)) match = true;

        if (!match) continue;

        list.push_back({
            {"id", u.id},
            {"username", u.username},
            {"avatar", u.avatar},
            {"bio", u.bio},
            {"post_count", u.post_count},
            {"like_count", u.like_count},
            {"follower_count", u.follower_count},
            {"following_count", u.following_count}
        });
    }

    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", list}
    });
}

void FollowUserHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[FollowUserHandler] 进入函数";
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code",1002},{"msg","unauthorized"}});
        return;
    }

    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    int targetId = 0;
    try { targetId = std::stoi(sid); } 
    catch (...) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    if (targetId == uid) {
        rsp->Json({{"code",2003},{"msg","cannot_follow_self"}});
        return;
    }

    // 加载用户
    std::vector<User> users;
    storage->LoadUsers(users);

    bool userExists = false;
    for (auto &u: users) if (u.id == targetId) userExists = true;

    if (!userExists) {
        rsp->Json({{"code",2001},{"msg","user_not_found"}});
        return;
    }

    // 加载关注记录
    std::vector<Follow> follows;
    storage->LoadFollows(follows);

    // 是否已关注
    for (auto &f : follows) {
        if (f.follower_id == uid && f.following_id == targetId) {
            rsp->Json({{"code",0},{"msg","already_follow"}});
            return;
        }
    }

    // 添加关注记录
    Follow f;
    f.follower_id = uid;
    f.following_id = targetId;
    f.time = time(NULL);

    follows.push_back(f);
    storage->SaveFollows(follows);
    AddNotification(targetId, uid, "follow", 0);


    // 更新用户统计
    for (auto &u: users) {
        if (u.id == uid) u.following_count++;
        if (u.id == targetId) u.follower_count++;
    }
    storage->SaveUsers(users);

    rsp->Json({
        {"code",0},
        {"msg","success"},
        {"data",{
            {"follower_id", uid},
            {"following_id", targetId}
        }}
    });
}

void UnfollowUserHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UnfollowUserHandler] 进入函数";
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code",1002},{"msg","unauthorized"}});
        return;
    }

    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    int targetId = 0;
    try { targetId = std::stoi(sid); } 
    catch (...) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    std::vector<Follow> follows;
    storage->LoadFollows(follows);

    bool found = false;
    for (auto it = follows.begin(); it != follows.end(); ++it) {
        if (it->follower_id == uid && it->following_id == targetId) {
            follows.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        rsp->Json({{"code",0},{"msg","not_follow"}});
        return;
    }

    storage->SaveFollows(follows);

    // 更新用户统计
    std::vector<User> users;
    storage->LoadUsers(users);

    for (auto &u: users) {
        if (u.id == uid && u.following_count > 0) u.following_count--;
        if (u.id == targetId && u.follower_count > 0) u.follower_count--;
    }
    storage->SaveUsers(users);

    rsp->Json({
        {"code",0},
        {"msg","success"},
        {"data",{
            {"follower_id", uid},
            {"unfollow_user", targetId}
        }}
    });
}

void UserFollowersHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserFollowersHandler] 进入函数";
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    int uid = 0;
    try { uid = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    std::vector<Follow> follows;
    storage->LoadFollows(follows);

    std::vector<User> users;
    storage->LoadUsers(users);

    json list = json::array();

    for (auto &f : follows) {
        if (f.following_id == uid) {
            for (auto &u : users) {
                if (u.id == f.follower_id) {
                    list.push_back({
                        {"id", u.id},
                        {"username", u.username},
                        {"avatar", u.avatar},
                        {"bio", u.bio}
                    });
                    break;
                }
            }
        }
    }

    rsp->Json({
        {"code",0},
        {"msg","success"},
        {"data", list}
    });
}

void UserFollowingHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UserFollowingHandler] 进入函数";
    std::string sid = req.GetParam("id");
    if (sid.empty()) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    int uid = 0;
    try { uid = std::stoi(sid); }
    catch (...) {
        rsp->Json({{"code",1001},{"msg","invalid_param"}});
        return;
    }

    std::vector<Follow> follows;
    storage->LoadFollows(follows);

    std::vector<User> users;
    storage->LoadUsers(users);

    json list = json::array();

    for (auto &f : follows) {
        if (f.follower_id == uid) {
            for (auto &u : users) {
                if (u.id == f.following_id) {
                    list.push_back({
                        {"id", u.id},
                        {"username", u.username},
                        {"avatar", u.avatar},
                        {"bio", u.bio}
                    });
                    break;
                }
            }
        }
    }

    rsp->Json({
        {"code",0},
        {"msg","success"},
        {"data", list}
    });
}

void PostCreateHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[PostCreateHandler] 进入函数";
    // 1. 登录校验
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 解析 JSON body
    json body;
    try {
        body = json::parse(req._body);
    } catch (...) {
        rsp->Json({{"code", 1001}, {"msg", "invalid json"}});
        return;
    }

    std::string title   = body.value("title", "");
    std::string content = body.value("content", "");
    std::vector<std::string> images = body.value("images", std::vector<std::string>{});
    std::vector<std::string> tags   = body.value("tags", std::vector<std::string>{});

    if (title.empty() || content.empty()) {
        rsp->Json({{"code", 1001}, {"msg", "title or content empty"}});
        return;
    }

    // 3. 读取帖子列表
    std::vector<Post> posts;
    storage->LoadPosts(posts);

    // 4. 分配 post_id
    int newId = posts.empty() ? 1 : (posts.back().id + 1);

    // 5. 构造 Post 对象
    Post post;
    post.id = newId;
    post.author_id = uid;
    post.title = title;
    post.content = content;
    post.images = images;
    post.tags = tags;
    post.create_time = time(NULL);

    post.like_count = 0;
    post.comment_count = 0;
    post.fav_count = 0;

    posts.push_back(post);

    // 6. 保存 posts.json
    if (!storage->SavePosts(posts)) {
        rsp->Json({{"code", 5001}, {"msg", "save_failed"}});
        return;
    }

    // 7. 用户发帖数++
    std::vector<User> users;
    storage->LoadUsers(users);

    for (auto &u : users) {
        if (u.id == uid) {
            u.post_count++;
            break;
        }
    }
    storage->SaveUsers(users);

    // 8. 返回前端
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {
            {"id", post.id},
            {"author_id", post.author_id},
            {"title", post.title},
            {"content", post.content},
            {"images", post.images},
            {"tags", post.tags},
            {"create_time", post.create_time},
            {"like_count", post.like_count},
            {"comment_count", post.comment_count},
            {"fav_count", post.fav_count}
        }}
    });
}

void NotificationListHandler(const http::HttpRequest& req, http::HttpResponse* rsp)
{
    LOG_DEBUG << "[NotificationListHandler] 进入函数";
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code",1002},{"msg","unauthorized"}});
        return;
    }

    std::vector<Notification> arr;
    storage->LoadNotifications(arr);

    json list = json::array();
    for (auto &n : arr) {
        if (n.uid == uid) {
            list.push_back({
                {"id", n.id},
                {"sender_id", n.sender_id},
                {"type", n.type},
                {"post_id", n.post_id},
                {"content", n.content},
                {"time", n.time},
                {"read", n.read}
            });
        }
    }

    // 让最新通知排前面
    std::sort(list.begin(), list.end(), [](const json& a, const json& b){
        return a["time"].get<long long>() > b["time"].get<long long>();
    });

    rsp->Json({{"code",0},{"msg","success"},{"data",list}});
}

void NotificationReadHandler(const http::HttpRequest& req, http::HttpResponse* rsp)
{
    LOG_DEBUG << "[NotificationReadHandler] 进入函数";
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code",1002},{"msg","unauthorized"}});
        return;
    }

    json body = json::parse(req._body, nullptr, false);
    int nid = body.value("id", 0);

    std::vector<Notification> arr;
    storage->LoadNotifications(arr);

    bool found = false;
    for (auto &n : arr) {
        if (n.id == nid && n.uid == uid) {
            n.read = true;
            found = true;
            break;
        }
    }

    if (!found) {
        rsp->Json({{"code",3002},{"msg","notification_not_found"}});
        return;
    }

    storage->SaveNotifications(arr);

    rsp->Json({{"code",0},{"msg","success"}});
}

void NotificationReadAllHandler(const http::HttpRequest& req, http::HttpResponse* rsp)
{
    LOG_DEBUG << "[NotificationReadAllHandler] 进入函数";
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code",1002},{"msg","unauthorized"}});
        return;
    }

    std::vector<Notification> arr;
    storage->LoadNotifications(arr);

    for (auto &n : arr) {
        if (n.uid == uid) {
            n.read = true;
        }
    }

    storage->SaveNotifications(arr);

    rsp->Json({{"code",0},{"msg","success"}});
}

void UploadImageHandler(const http::HttpRequest &req, http::HttpResponse *rsp)
{
    LOG_DEBUG << "[UploadImageHandler] 进入函数";

    // 1. 登录校验
    int uid = Auth(req);
    if (uid < 0) {
        rsp->Json({{"code", 1002}, {"msg", "unauthorized"}});
        return;
    }

    // 2. 检查是否 multipart/form-data
    std::string contentType = req.GetHeader("Content-Type");
    std::string boundaryKey = "boundary=";
    size_t bpos = contentType.find(boundaryKey);
    if (bpos == std::string::npos) {
        rsp->Json({{"code", 1001}, {"msg", "invalid multipart"}});
        return;
    }
    std::string boundary = contentType.substr(bpos + boundaryKey.size());

    // 3. 使用你自己的 MultipartParser（最规范的方式）
    MultipartParser parser(req._body, boundary);
    if (!parser.Parse()) {
        rsp->Json({{"code", 1001}, {"msg", "parse failed"}});
        return;
    }

    if (!parser.HasFile("file")) {
        rsp->Json({{"code", 1001}, {"msg", "file missing"}});
        return;
    }

    MultipartFile mf = parser.GetFile("file");

    // 4. 提取文件后缀
    std::string ext = ".jpg";
    size_t dot = mf.filename.find_last_of('.');
    if (dot != std::string::npos) {
        ext = mf.filename.substr(dot);
    }

    // 5. 生成唯一文件名（与你项目风格一致）
    char timeBuf[32];
    time_t now = time(NULL);
    strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S_", localtime(&now));

    static const char alphas[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::mt19937 rng((unsigned)time(NULL));
    std::string rnd;
    for (int i = 0; i < 8; i++) rnd += alphas[rng() % (sizeof(alphas)-1)];

    std::string newName = std::string(timeBuf) + rnd + ext;

    // 6. 保存目录
    std::string saveDir = "../wwwroot/static/upload";
    FileUtil::CreateDirIfNotExists(saveDir);   // 用你自己的目录工具

    std::string savePath = saveDir + "/" + newName;

    // 7. 保存文件（使用你的 FileUtil）
    {
        std::ofstream ofs(savePath, std::ios::binary);
        if (!ofs.is_open()) {
            rsp->Json({{"code", 5001}, {"msg", "save_failed"}});
            return;
        }
        ofs.write(mf.data.data(), mf.data.size());
    }

    // 8. 生成 URL（与你现有 static 路径一致）
    std::string url = "/static/upload/" + newName;

    // 9. 返回 JSON
    rsp->Json({
        {"code", 0},
        {"msg", "success"},
        {"data", {{"url", url}}}
    });
}