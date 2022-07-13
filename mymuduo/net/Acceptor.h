#pragma once

#include <functional>

#include "mymuduo/utils/noncopyable.h"
#include "mymuduo/net/Socket.h"
#include "mymuduo/net/Channel.h"

class EventLoop;
class InetAddress;

// Acceptor MainReactor拥有主fd管理新连接到达
// 新连接到达后通过调用server上的回调建立新连接
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) { NewConnectionCallback_ = cb; }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop *loop_; // Acceptor用的就是用户定义的那个baseLoop 也称作mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback NewConnectionCallback_;
    bool listenning_;
};