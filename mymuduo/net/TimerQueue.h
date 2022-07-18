#pragma once
#include <set>
#include <vector>
#include "mymuduo/utils/Timestamp.h"
#include "mymuduo/net/Callbacks.h"
#include "mymuduo/net/Channel.h"
class EventLoop;
class Timer;
class TimerId;

using TimerCallback = std::function<void()>;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue
{
public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  // 加入定时器，返回id跟踪
  TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
  // 取消定时器，根据id
  void cancel(TimerId timerId);

 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  // 绑定定时时间和定时器指针
  typedef std::pair<Timestamp, Timer*> Entry;
  // 最小堆
  typedef std::set<Entry> TimerList;
  // 
  typedef std::pair<Timer*, int64_t> ActiveTimer;
  // 最x堆
  typedef std::set<ActiveTimer> ActiveTimerSet;

  // 在loop加入定时器 和取消
  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  // 定时器到了需要处理
  void handleRead();
  // move out all expired timers
  // 看看哪个定时器超时
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;

  // for cancel()
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_; /* atomic */
  ActiveTimerSet cancelingTimers_;
};
#pragma once