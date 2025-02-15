#pragma

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Connection;
class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    HttpRequest() : method(""), path(""), version(""), body("") {}
};

// 函数声明
std::vector<std::string> split(const std::string& str, char delimiter);
HttpRequest parseHttpRequest(const std::string& requestData);
std::string getContentType(const std::string& filePath);
void handleClient(int sockfd, const HttpRequest& request, Connection* conn);
