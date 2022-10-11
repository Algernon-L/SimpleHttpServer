# SimpleHttpServer
基于 muduo 网络库的轻量级服务器
采用 non-blocking IO + one loop per thread + thread pool 设计思想
- 主 Reactor 响应服务端新到链接请求，子 Reactor 监听链接注册的 IO 事件。
- 使用自动增长的缓冲区收发字节流，配合 epoll LT 模式确保数据读写成功。
- 利用线程池管理 IO 线程和计算线程，同时实现数据库链接池管理链接。
- 实现定时器，通过 timerfd 纳入 epoll，统一管理定时事件和 IO 事件。
- 使用主从状态机解析 HTTP 报文，支持响应 GET 和 POST 请求。
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
