#include "../http/HttpServer.hpp"
#include "WebHandler.hpp"

using namespace webserver;

#define WWWROOT "../wwwroot/"
static std::string ConvertRoute(const std::string& route)
{
    std::string re = "^";
    for (size_t i = 0; i < route.size(); ++i) {
        if (route[i] == '{') {
            // 找参数结尾
            size_t j = route.find('}', i);
            if (j == std::string::npos) throw std::runtime_error("bad route");

            // 参数名 = route.substr(i+1, j-i-1); // 如果你未来需要用
            re += "([0-9]+)";  // 目前全部匹配数字 id
            i = j;
        } else {
            // 正常字符，转义 regex 中的特殊符号
            if (std::string(".^$|()[]*+?\\").find(route[i]) != std::string::npos)
                re += "\\";
            re += route[i];
        }
    }
    re += "/?$";
    return re;
}

void RegisterRoutes(http::HttpServer& server)
{
    storage = new JsonStorage("../data");

    // ===== User Auth =====
    server.Post( ConvertRoute("/user/register"), RegisterHandler );
    server.Post( ConvertRoute("/user/login"),    LoginHandler );

    // ===== User Self Update / Upload =====
    server.Post( ConvertRoute("/user/profile/update"), UserUpdateHandler );
    server.Post( ConvertRoute("/user/avatar/upload"),  AvatarUploadHandler );

    // ==== Upload ====
    server.Post( "/upload/image", UploadImageHandler );
    // ===== Posts =====
    server.Get ( ConvertRoute("/posts"),            PostListHandler );
    server.Post( ConvertRoute("/post/create"),      PostCreateHandler );
    server.Get ( ConvertRoute("/post/{id}"),        PostDetailHandler );
    server.Post( ConvertRoute("/post/{id}/comment"), CommentCreateHandler );
    server.Get ( ConvertRoute("/post/{id}/comments"), CommentListHandler );

    server.Post( ConvertRoute("/post/{id}/like"),    LikePostHandler );
    server.Post( ConvertRoute("/post/{id}/unlike"),  UnlikePostHandler );
    server.Post( ConvertRoute("/post/{id}/fav"),      FavPostHandler );
    server.Post( ConvertRoute("/post/{id}/unfav"),    UnfavPostHandler );


    // ===== Follow System =====
    server.Post( ConvertRoute("/user/{id}/follow"),   FollowUserHandler );
    server.Post( ConvertRoute("/user/{id}/unfollow"), UnfollowUserHandler );
    server.Get ( ConvertRoute("/user/{id}/followers"), UserFollowersHandler );
    server.Get ( ConvertRoute("/user/{id}/following"), UserFollowingHandler );


    // ===== Recommendation =====
    server.Get( ConvertRoute("/topics/hot"),        HotTopicsHandler );
    server.Get( ConvertRoute("/recommend/posts"),   RecommendPostsHandler );
    server.Get( ConvertRoute("/recommend/creators"),RecommendCreatorsHandler );


    // ===== Search =====
    server.Get( ConvertRoute("/search/posts"), SearchPostsHandler );
    server.Get( ConvertRoute("/search/users"), SearchUsersHandler );


    // ===== Notifications =====
    server.Get ( ConvertRoute("/notifications"),              NotificationListHandler );
    server.Post( ConvertRoute("/notifications/{id}/read"),    NotificationReadHandler );
    server.Post( ConvertRoute("/notifications/read_all"),     NotificationReadAllHandler );


    // ===== User Profile (放在最后，因为带参数) =====
    server.Get( ConvertRoute("/user/{id}"), UserProfileHandler );

}

int main() 
{
    EnableConsoleSink();
    http::HttpServer server(8080);
    server.SetThreadCount(0);
    server.SetBaseDir(WWWROOT);
    RegisterRoutes(server);
    server.Listen();

    return 0;
}