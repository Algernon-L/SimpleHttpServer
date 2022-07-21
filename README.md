# SimpleHttpServer
基于Muduo C++网络库的简易版HTTP服务器

采用non-blocking IO + one loop per thread + thread pool 的设计思想
- 主Reactor响应服务端新到连接请求，子Reactor监听连接注册的IO事件。
- 使用自动增长的应用层发送/接收双缓冲区，配合epoll LT模式确保数据读写成功。
- 定时器使用set容器管理，通过timerfd纳入epoll，统一管理定时事件和IO事件。
- 使用主从状态机解析HTTP报文，支持响应GET和POST请求。
- 通过数据库操作实现用户注册和登录，可以请求服务器上的文件。

## 开发环境

* linux kernel version 5.13.0 (ubuntu 20.04 Server)
* gcc version 11.1.0
* cmake version 3.16.3

## 编译执行
```shell
sudo ./build.sh
cd example
./build.sh
./webserver.o [线程数量] [监听的ip] [监听的端口]
```

### References
[[1]](https://github.com/chenshuo/muduo)Event-driven network library for multi-threaded Linux server in C++11, **Chen shuo**<br/>
[[2]](https://github.com/qinguoyi/TinyWebServer)Linux下C++轻量级Web服务器,  **Qin guoyi**