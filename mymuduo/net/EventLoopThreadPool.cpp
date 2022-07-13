#include <memory>

#include "mymuduo/net/EventLoopThreadPool.h"
#include "mymuduo/net/EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;
    // 准备固定大小的线程池
    for(int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        // 将cb和线程池名字传入Thread类
        // 先调用EventLoopThread构造函数建立新线程
        EventLoopThread *t = new EventLoopThread(cb, buf);
        // 集中管理
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // 新建线程->执行EventLoopThread::ThreadFunc()->新建EventLoop->同步绑定线程和循环
        loops_.push_back(t->startLoop());
    }

    if(numThreads_ == 0 && cb)// 没有线程池的情况
    {
        cb(baseLoop_);
    }
}

// 如果工作在多线程中，baseLoop_(mainLoop)会默认以轮询的方式分配Channel给subLoop
EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;    // 如果只设置一个线程 也就是只有一个mainReactor 无subReactor 那么轮询只有一个线程 getNextLoop()每次都返回当前的baseLoop_

    if(!loops_.empty())             // 通过轮询获取下一个处理事件的loop
    {
        loop = loops_[next_];
        ++next_;
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}