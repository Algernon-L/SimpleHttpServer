#include <functional>
#include <string.h>

#include "mymuduo/net/TcpServer.h"
#include "mylogger/Logger.h"
#include "mymuduo/net/TcpConnection.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL << "mainLoop is nullptr!";
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
    : loop_(CheckLoopNotNull(loop))
    , ipPort_(listenAddr.toIpPort())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    , threadPool_(new EventLoopThreadPool(loop, name_))
    , connectionCallback_()
    , messageCallback_()
    , nextConnId_(1)
    , started_(0)
{
    // 新连接由acceptor中的epoll返回，调用回调函数让server中获取线程建立连接。
    // 当有新用户连接时，Acceptor类中绑定的acceptChannel_会有读事件发生，执行handleRead()调用TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for(auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();    // 把原始的智能指针复位 让栈空间的TcpConnectionPtr conn指向该对象 当conn出了其作用域 即可释放智能指针指向的对象
        // 销毁连接
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

// 设置底层subloop的个数
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

// 开启服务器监听
// 1.初始化baseloop
// 2.在baseloop运行acceptor
void TcpServer::start()
{
    if (started_++ == 0)    // 防止一个TcpServer对象被start多次
    {
        threadPool_->start(threadInitCallback_);    // 启动底层的loop线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

// 有一个新用户连接，acceptor会执行这个回调操作，负责将mainLoop接收到的请求连接(acceptChannel_会有读事件发生)通过回调轮询分发给subLoop去处理
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    // 1.轮询算法 选择一个subLoop 来管理connfd对应的channel
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;  // 这里没有设置为原子类是因为其只在mainloop中执行 不涉及线程安全问题
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_ <<"] - new connection [" << connName << "] from " << peerAddr.toIpPort();
    
    // 2.通过sockfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if(::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
    {
        LOG_ERROR << "sockets::getLocalAddr";
    }
    // 3.建立新连接并存储到server中
    InetAddress localAddr(local);
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    connections_[connName] = conn;
    // 4.用户自定义conn的三个回调，从server传到conn
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    // 设置了如何关闭连接的回调
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 5.在线程集合中的某个线程管理连接
    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn));
}

// 移除指定连接
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

// 移除某个线程中的所有连接
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO << "TcpServer::removeConnectionInLoop ["<<name_<<"] - connection "<<conn->name();

    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}