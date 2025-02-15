#include "Acceptor.h"
#include "Socket.h"
#include "../Epoll/Channel.h"
#include "../Log/Log.h"
#include <cstdio>

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop), sock(nullptr), acceptChannel(nullptr){
    sock = new Socket();
    InetAddress *addr = new InetAddress("192.168.233.143", 9001);
    sock->bind(addr);
    sock->listen(); 
    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setReadCallback(cb);
    acceptChannel->enableRead();
    delete addr;
}

Acceptor::~Acceptor(){
    delete sock;
    delete acceptChannel;
}

void Acceptor::acceptConnection(){
    InetAddress *clnt_addr = new InetAddress();      
    Socket *clnt_sock = new Socket(sock->accept(clnt_addr));
    LOG_INFO("New client");  
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), clnt_addr->getIp(), clnt_addr->getPort());
    clnt_sock->setnonblocking();  //新接受到的连接设置为非阻塞式
    newConnectionCallback(clnt_sock);
    delete clnt_addr;
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb){
    newConnectionCallback = _cb;
}