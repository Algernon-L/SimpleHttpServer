# socket
```cpp
// 1.socket
int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);

// 2.bind
int bind(int sockfd, const struct sockaddr * addr, socklen_t addrlen);

// 3. 
int listen(int sockfd, int backlog);

// 4.
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

IO复用

```cpp
int epoll_create(int size);
int epoll_create1(EPOLL_CLOEXEC);
// size 本来用于指定epollfd监听的数量，用于分配epoll数据结构。

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
// 用于增删改 关注的文件描述符和事件。
    typedef union epoll_data {
        void        *ptr;
        int          fd;
        uint32_t     u32;
        uint64_t     u64;
    } epoll_data_t;

    struct epoll_event {
        uint32_t     events;      /* Epoll events */
        epoll_data_t data;        /* User data variable */
    };
/*
EPOLLIN fd可以执行read操作
EPOLLOUT fd可以执行write操作
EPOLLRDHUP 流socket的对方关闭了连接/关闭了写端（在ET模式下用于检测对方是否shutdown）
EPOLLERR 不需要手动设置，发生了错误，如对端关闭读时写端报ERR。
EPOLLHUP 不需要手动设置，提示对方已经关闭通道，随后read会返回0
EPOLLET 默认LT，
EPOLLONESHOT 仅提醒一次
*/

int epoll_wait(int epfd, struct epoll_event *events,
            int maxevents, int timeout);
/*
返回的三种情况：
1.有关注的事件发生
2.被信号打断
3.超时

返回值：
1.>0 存在就绪事件的文件描述符数量
2.==0 超时
3.-1 错误

就绪文件描述符超过了maxevents？
会进行负载均衡，防止饥饿
*/
```

```cpp
// epoll Q & A
/*
1.同一个epoll实例，重复注册相同的文件描述符？
很可能得到错误代码EEXIST，但是你可以通过dup添加重复的文件描述符（但关注事件不同）到相同的epoll实例，这可以作为一种筛选事件的方法。

2.两个epoll实例监听同一个fd？
两个epoll都会收到通知，但需要谨慎地coding。

3.epollfd本身可以被select/poll/epoll吗？
可以，如果epollfd正在等待事件，他会被提示可读。

4.epollfd注册自己本身？
返回EINVAL

5.被注册的fd调用close，epoll有什么反应？

6.读写事件同时就绪，会分开提示还是合并？
合并

7.

8.ET造成饥饿？
某个fd大量读写，导致其他就绪fd饥饿。
使用就绪队列，然后进行负载均衡，这也可以帮助你忽略之后就绪的fd

9.使用event cache

*/
```

ET and LT
```cpp
/*
ET编程推荐做法？
1.使用NON-BLOCKING fd
2.就绪事件需要read或write至EAGAIN

ET比LT高效？
节省切换EPOLLIN or EPOLLOUT开关的EPOLL_CTL_MOD调用
*/
```

readv 和 read
```cpp
/*
readv 可以设置多个缓冲区
read 只能设置一个缓冲区
*/
```