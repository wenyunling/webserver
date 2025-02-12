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
    std::string requestData;
public:
    Connection(EventLoop *_loop, Socket *_sock);
    ~Connection();
    
    void context(int sockfd);
    void setDeleteConnectionCallback(std::function<void(int)>);
};

