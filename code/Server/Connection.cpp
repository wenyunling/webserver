#include "Connection.h"
#include "Socket.h"
#include "../Epoll/Channel.h"
#include "../Util/util.h"
#include "../Buffer/Buffer.h"
#include "../Http/HttpService.h"
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>


Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr), readBuffer(nullptr), requestData("error"){
    channel = new Channel(loop, sock->getFd());
    channel->enableRead();
    channel->useET();
    std::function<void()> cb = std::bind(&Connection::readFile, this, sock->getFd());
    channel->setReadCallback(cb);
    readBuffer = new Buffer();
    channel->setWriteCallback([this]() {
        if (isSending) {
            trySendFile(); // 继续发送剩余数据
        }
    });
}

Connection::~Connection(){
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> _cb){
    deleteConnectionCallback = _cb;
}



void Connection::readFile(int sockfd) {
    char buf[1024];  
    while (true) {   
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            readBuffer->append(buf, bytes_read);
        } else if (bytes_read == -1 && errno == EINTR) {  // 客户端正常中断、继续读取
            printf("continue reading\n");
            continue;
        } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  //数据全部读取完毕
            requestData = readBuffer->c_str();
            readBuffer->clear();
            printf("message from client fd %d \n", sockfd);

            HttpRequest request = parseHttpRequest(requestData);
            handleClient(sockfd, request,this);

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


// Connection.cpp
void Connection::startSending(const std::string& filePath) {
    sendFilePath = filePath;
    sendFileOffset = 0;
    isSending = true;
    trySendFile(); // 首次尝试直接发送
}

void Connection::trySendFile() {
    if (!isSending) return;

    std::ifstream file(sendFilePath, std::ios::binary);
    if (!file) {
        deleteConnectionCallback(sock->getFd());         
        return;
    }

    file.seekg(sendFileOffset);
    const size_t buffer_size = 1024 * 64; 
    char buffer[buffer_size];

    while (isSending) {
        file.read(buffer, buffer_size);
        ssize_t bytes_read = file.gcount();
        if (bytes_read <= 0) {
            isSending = false; 
            channel->disableWrite(); 
            break;
        }

        ssize_t bytes_sent = 0;
        while (bytes_sent < bytes_read) {
            ssize_t ret = write(sock->getFd(), buffer + bytes_sent, bytes_read - bytes_sent);
            if (ret == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    channel->enableWrite(); 
                    sendFileOffset += bytes_sent; // 记录已发送量
                    return;
                } else {
                    deleteConnectionCallback(sock->getFd());  
                    return;
                }
            }
            bytes_sent += ret;
        }
        sendFileOffset += bytes_sent; // 更新
    }

    file.close();
}