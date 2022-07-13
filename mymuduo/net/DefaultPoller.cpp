#include <stdlib.h>

#include "mymuduo/net/Poller.h"
#include "mymuduo/net/EPollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; // 生成poll的实例
    }
    else
    {
        return new EPollPoller(loop); // 生成epoll的实例
    }
}