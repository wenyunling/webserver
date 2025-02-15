## Tiny Webserver

### 更新
  ```
  [2025.2.16]
  更新了connection类和channel类，使得服务器支持了非阻塞写操作，现在服务器可以提供视频和图片的访问
  ```
  
### 项目概述：
这是一个用 C++ 实现的轻量级 Web 服务器，运行在 Linux 环境下。项目采用了 **主从 Reactor** 模式，处理客户端请求的同时，确保高效地处理多个连接。该服务器的基本功能是提供 HTTP 服务，并支持异步日志、线程池处理请求等功能。

### 运行方式：
1. 使用 `make server` 编译项目。
2. 运行 `./server` 启动服务器。
3. 通过浏览器访问 `http://<IP>:9001`，即可查看服务器响应。默认情况下，服务器会监听本地的 9001 端口，可以通过修改 `code/Server/Acceptor.cpp` 中的配置修改监听地址。

---

### 项目文件结构：

```
.
├── code
│   ├── Buffer
│   │   ├── Buffer.cpp
│   │   └── Buffer.h
│   ├── Epoll
│   │   ├── Channel.cpp
│   │   ├── Channel.h
│   │   ├── Epoll.cpp
│   │   └── Epoll.h
│   ├── Http
│   │   ├── HttpService.cpp
│   │   └── HttpService.h
│   ├── Log
│   │   ├── BlockQueue.h
│   │   ├── Log.cpp
│   │   └── Log.h
│   ├── Server
│   │   ├── Acceptor.cpp
│   │   ├── Acceptor.h
│   │   ├── Connection.cpp
│   │   ├── Connection.h
│   │   ├── EventLoop.cpp
│   │   ├── EventLoop.h
│   │   ├── Server.cpp
│   │   ├── Server.h
│   │   ├── Socket.cpp
│   │   └── Socket.h
│   ├── ThreadPool
│   │   ├── ThreadPool.cpp
│   │   └── ThreadPool.h
│   └── Util
│       ├── util.cpp
│       └── util.h
├── logs
└── source
    ├── index.html
    ├── picture.png
    └── video.mp4
├── Makefile
├── Readme.md
└── server.cpp
```

---

### 各模块功能：

#### 1. **Buffer**
   - **功能**：该模块实现了一个自动增长的缓冲区，用来缓存从客户端接收到的数据，或者准备发送到客户端的数据。
#### 2. **Epoll**
   - **功能**：封装了 Linux `epoll` 的使用，提供高效的事件通知机制来处理大量并发连接。该模块是实现非阻塞 I/O 的核心，支持同时处理多个客户端连接。

#### 3. **Http**
   - **功能**：该模块负责 HTTP 协议的解析和响应的生成，包括请求的解析、响应报文的构建等。

#### 4. **Log**
   - **功能**：提供异步日志功能，用于记录服务器运行中的重要信息、错误信息等。


#### 5. **Server**
   - **功能**：该模块实现了 Web 服务器的主要功能，包括服务器初始化、事件循环、请求的接受和处理等。通过 `Acceptor` 类接受客户端连接，并交给工作线程进行处理。
     - `Acceptor` 类负责监听指定端口并接受新的客户端连接。
     - `EventLoop` 类负责事件的分发和调度，处理 `epoll` 事件，调用适当的回调函数。
     - `Connection` 类负责管理与客户端的连接，读取数据、发送响应等。
     - `Server` 类是整个服务器的核心，负责初始化、启动和管理各个模块的工作。

#### 6. **ThreadPool**
   - **功能**：实现一个简单的线程池，用于处理客户端的请求。每当一个连接被接受后，将该请求分配给线程池中的一个线程进行处理。


#### 7. **Util**
   - **功能**：该模块包含一工具函数。


