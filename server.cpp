#include "code/Server/EventLoop.h"
#include "code/Server/Server.h"
#include "code/ThreadPool/ThreadPool.h"
#include "code/Log/Log.h"


int main() {
    Log::Instance().Init(LogLevel::DEBUG,
                         "./logs", 
                         1024*1024);    
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    loop->loop();

    return 0;
}
