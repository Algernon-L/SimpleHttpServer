set(net_SRCS
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    DefaultPoller.cpp
    EPollPoller.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    InetAddress.cpp
    Poller.cpp
    Socket.cpp
    TcpConnection.cpp
    TcpServer.cpp
    Thread.cpp
    Timer.cpp
    TimerQueue.cpp
)

add_library(mymuduo_net ${net_SRCS})
target_link_libraries(mymuduo_net mymuduo_utils)

install(TARGETS mymuduo_net DESTINATION lib)
set(HEADERS
    Acceptor.h
    Buffer.h
    Callbacks.h
    Channel.h
    EPollPoller.h
    EventLoop.h
    EventLoopThread.h
    EventLoopThreadPool.h
    InetAddress.h
    Poller.h
    Socket.h
    TcpConnection.h
    TcpServer.h
    Thread.h
    Timer.h
    TimerId.h
    TimerQueue.h
)

install(FILES ${HEADERS} DESTINATION include/mymuduo/net)

