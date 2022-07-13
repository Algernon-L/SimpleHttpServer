#include "mymuduo/net/Thread.h"
#include "mymuduo/utils/CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

// 构造函数
Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)), name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach(); // thread类提供了设置分离线程的方法 线程运行后自动销毁（非阻塞）
    }
}

// 开启线程，执行工作函数
void Thread::start() // 一个Thread对象 记录的就是一个新线程的详细信息
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0); // false指的是 不设置进程间共享
    // 开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&]()
        {
        // 获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        // 开启一个新线程 专门执行该线程函数 
        func_(); 
        }));
    // 等待信号量，代表新建线程成功
    sem_wait(&sem);
}

// C++ std::thread 中join()和detach()的区别：https://blog.nowcoder.net/n/8fcd9bb6e2e94d9596cf0a45c8e5858a
void Thread::join()
{
    joined_ = true;
    thread_->join();
}

// 设置线程编号
void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}