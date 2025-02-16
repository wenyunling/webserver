#pragma once
#include <functional>
#include <string>
class EventLoop;
class Socket;
class Channel;
class Buffer;
class Connection
{
private:
    EventLoop *loop;
    Socket *sock;
    Channel *channel;
    std::function<void(int)> deleteConnectionCallback;
    Buffer *readBuffer;
    Buffer *sendBuffer;
    std::string requestData;

    std::string sendFilePath; // 待发送文件路径
    size_t sendFileOffset = 0; // 当前发送偏移量
    bool isSending = false; // 发送状态标记

    void trySendFile(); 
public:
    Connection(EventLoop *_loop, Socket *_sock);
    ~Connection();
    
    void readFile(int sockfd);

    void setDeleteConnectionCallback(std::function<void(int)>);
    void startSending(const std::string& filePath);
    void setRequestHandler(std::function<void(const std::string&, Connection*)> handler);

};

void writeNoBlocking(int sockfd, const std::string& filePath);

