#include "HttpService.h"
#include "../Server/Connection.h"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>
#include <filesystem>
#include <sstream>
#include <vector>
#include <cerrno>
#include <cerrno>
#include <cstring>

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
        return "application/octet-stream"; 
    }
}


void handleClient(int sockfd,const HttpRequest& request, Connection* conn) {
    // 路由表：请求路径 -> 资源文件路径
    std::unordered_map<std::string, std::string> routes = {
        {"/", "source/index.html"},
        {"/source/video.mp4", "source/video.mp4"},
        {"/source/picture.png", "source/picture.png"},
        {"/favicon.ico", "source/1.ico"}
    };

    std::string requestedPath = request.path;
    std::string response;

    // 处理请求路径
    if (routes.find(requestedPath) != routes.end()) {
        std::string filePath = routes[requestedPath]; 
        std::string contentType = getContentType(filePath);

        // 打开文件并读取内容
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            response = "HTTP/1.1 500 Internal Server Error\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: 21\r\n";
            response += "\r\n";
            response += "500 Internal Server Error";
            send(sockfd, response.c_str(), response.size(), 0);
        }
    
        // 获取文件大小
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
    
        // 构建响应头
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + contentType + "\r\n";
        response += "Content-Length: " + std::to_string(fileSize) + "\r\n";
        response += "\r\n";
        send(sockfd, response.c_str(), response.size(), 0);
        
        conn->startSending(filePath);
    
        file.close();    
    } else {
        // 资源未找到，返回 404 Not Found
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: 13\r\n";
        response += "\r\n";
        response += "404 Not Found";
        send(sockfd, response.c_str(), response.size(), 0);
    }
    
}

