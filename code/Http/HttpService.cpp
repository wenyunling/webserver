#include "HttpService.h"
#include <sstream>
#include <fstream>

namespace fs = std::filesystem;

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


HttpRequest parseHttpRequest(const std::string& requestData) {
    HttpRequest request;
    std::vector<std::string> lines = split(requestData, '\n');

    // 解析请求行
    std::vector<std::string> requestLine = split(lines[0], ' ');
    if (requestLine.size() >= 3) {
        request.method = requestLine[0];
        request.path = requestLine[1];
        request.version = requestLine[2];
    }

    // 解析请求头
    for (size_t i = 1; i < lines.size(); ++i) {
        std::string line = lines[i];
        if (line.empty()) {
            break;
        }
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);
            request.headers[key] = value;
        }
    }

    return request;
}

std::string getContentType(const std::string& filePath) {
    if (filePath.find(".html") != std::string::npos) {
        return "text/html";
    } else if (filePath.find(".css") != std::string::npos) {
        return "text/css";
    } else if (filePath.find(".js") != std::string::npos) {
        return "application/javascript";
    } else if (filePath.find(".png") != std::string::npos) {
        return "image/png";
    } else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) {
        return "image/jpeg";
    } else if (filePath.find(".gif") != std::string::npos) {
        return "image/gif";
    } else if (filePath.find(".bmp") != std::string::npos) {
        return "image/bmp";
    } else if (filePath.find(".mp4") != std::string::npos) {
        return "video/mp4";
    } else if (filePath.find(".webm") != std::string::npos) {
        return "video/webm";
    } else if (filePath.find(".ogg") != std::string::npos) {
        return "audio/ogg";
    } else if (filePath.find(".mp3") != std::string::npos) {
        return "audio/mp3";
    } else {
        return "application/octet-stream"; // Default binary type for unknown files
    }
}

std::string generateHttpResponse(const HttpRequest& request, const std::string& resourcePath) {
    std::string response;

    // 默认返回 index.html 如果路径是 "/"
    std::string filePath = resourcePath + (request.path == "/" ? "/index.html" : request.path);

    // 检查文件是否存在
    if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
        // 读取文件内容
        std::ifstream file(filePath, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // 生成响应头
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + getContentType(filePath) + "\r\n";
        response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
        response += "\r\n";
        response += content;
    } else {
        // 文件不存在，返回 404 Not Found
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: 13\r\n";
        response += "\r\n";
        response += "404 Not Found";
    }

    return response;
}

