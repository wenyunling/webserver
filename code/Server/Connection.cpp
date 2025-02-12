#include "Connection.h"
#include "Socket.h"
#include "../Epoll/Channel.h"
#include "../Util/util.h"
#include "../Buffer/Buffer.h"
#include "../Http/HttpService.h"
#include <unistd.h>
#include <string.h>
#include <iostream>

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr), readBuffer(nullptr), requestData("error"){
    channel = new Channel(loop, sock->getFd());
    channel->enableRead();
    channel->useET();
    std::function<void()> cb = std::bind(&Connection::context, this, sock->getFd());
    channel->setReadCallback(cb);
    readBuffer = new Buffer();
}

Connection::~Connection(){
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> _cb){
    deleteConnectionCallback = _cb;
}


void Connection::context(int sockfd) {
    char buf[1024];  // 这个buf大小无所谓
    while (true) {    // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            readBuffer->append(buf, bytes_read);
        } else if (bytes_read == -1 && errno == EINTR) {  // 客户端正常中断、继续读取
            printf("continue reading\n");
            continue;
        } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞IO，这个条件表示数据全部读取完毕
            requestData = readBuffer->c_str();
            readBuffer->clear();
            printf("message from client fd %d: %s\n", sockfd, requestData.c_str());

            HttpRequest request = parseHttpRequest(requestData);
            std::string resourcePath = "./source";
            std::string response = generateHttpResponse(request, resourcePath);

            // 分批发送数据直到发送完毕
            size_t total_sent = 0;
            size_t response_len = response.length();
            while (total_sent < response_len) {
                ssize_t bytes_sent = send(sockfd, response.c_str() + total_sent, response_len - total_sent, 0);
                if (bytes_sent == -1) {
                    perror("data send error");
                    break;
                }
                total_sent += bytes_sent;  
            }
            break;
        } else if (bytes_read == 0) {  // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sockfd);           
            break;
        } else {
            printf("Connection reset by peer\n");
            deleteConnectionCallback(sockfd);         
            break;
        }
    }
}

