#include "mymuduo/net/TimerQueue.h"
#include "mymuduo/net/EventLoop.h"
#include "mymuduo/net/Timer.h"
#include "mymuduo/net/TimerId.h"
#include "mylogger/Logger.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <string>

using TimerCallback = std::function<void()>;

// 定时器fd 每个loop一个
int createTimerfd()
{
  // 文件描述符在定时器超时的那一刻变得可读，可以纳入epoll
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    LOG_FATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

// 剩余时间
// ret 离超时还有多久
struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

// 读定时器fd
void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_INFO << "TimerQueue::handleRead() "<<howmany<<" at "<<now.toString();
  if (n != sizeof howmany)
  {
    LOG_ERROR << "TimerQueue::handleRead() reads "<<n<<"bytes instead of 8";
  }
}

// 设置新的超时时间
void resetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, 0, sizeof newValue);
  memset(&oldValue, 0, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    LOG_ERROR << "timerfd_settime()";
  }
}

// 构造函数，绑定readCb，启动监听
TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers_(false)
{
  timerfdChannel_.setReadCallback(
      std::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

// 析构函数
// 取消所有关注事件，移除channel，关闭fd，删除timer
TimerQueue::~TimerQueue()
{
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (const Entry& timer : timers_)
  {
    delete timer.second;
  }
}

// 加定时器到定时器队列
// 返回TimerID 跟踪
TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval)
{
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->runInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}

// 在队列中取消定时器
void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(
      std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

// loop中的操作
// 看看最短超时的定时器是否变化了
// 变化则重置Timerfd的超时事件为最新定时器
void TimerQueue::addTimerInLoop(Timer* timer)
{
  if(!loop_->isInLoopThread()){

  }
  bool earliestChanged = insert(timer);

  if (earliestChanged)
  {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

// 在loop中取消定时器
void TimerQueue::cancelInLoop(TimerId timerId)
{
  if(!loop_->isInLoopThread()){
    
  }
  // 根据id找到定时器
  ActiveTimer timer(timerId.timer_, timerId.sequence_);
  ActiveTimerSet::iterator it = activeTimers_.find(timer);
  // 找到了
  if (it != activeTimers_.end())
  {
    size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
    delete it->first; // FIXME: no delete please
    activeTimers_.erase(it);
  }// 找不到但正在处理超时定时器，那么可能被移动了。下次取消他
  else if (callingExpiredTimers_)
  {
    cancelingTimers_.insert(timer);
  }
}

// 主要函数，处理定时器超时
void TimerQueue::handleRead()
{
  if(!loop_->isInLoopThread()){
    
  }
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);

  // 获得超时的定时器
  std::vector<Entry> expired = getExpired(now);

  callingExpiredTimers_ = true;
  cancelingTimers_.clear();
  // safe to callback outside critical section
  for (const Entry& it : expired)
  {
    it.second->run();//timer.callback()
  }
  callingExpiredTimers_ = false;

  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
  std::vector<Entry> expired;
  //reinterpret_cast<Timer*>(UINTPTR_MAX) 定义无效指针
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  // 获得离现在最近的定时器
  TimerList::iterator end = timers_.lower_bound(sentry);
  // [begin, end)插入到expired
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);

  for (const Entry& it : expired)
  {
    ActiveTimer timer(it.second, it.second->sequence());
    size_t n = activeTimers_.erase(timer);
  }

  return expired;
}
/*
如果需要重复，则重新设置超时时间然后插入定时器
如果此次超时定时器全部处理完成后，又有新一轮的定时器，重设超时时间
*/
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nextExpire;

  for (const Entry& it : expired)
  {
    ActiveTimer timer(it.second, it.second->sequence());
    if (it.second->repeat()
        && cancelingTimers_.find(timer) == cancelingTimers_.end())
    {
      it.second->restart(now);
      insert(it.second);
    }
    else
    {
      // FIXME move to a free list
      delete it.second; // FIXME: no delete please
    }
  }

  if (!timers_.empty())
  {
    nextExpire = timers_.begin()->second->expiration();
  }

  if (nextExpire.valid())
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer* timer)
{
  if(!loop_->isInLoopThread()){
    
  }
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first)
  {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result
      = timers_.insert(Entry(when, timer));
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result
      = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
  }
  return earliestChanged;
}

