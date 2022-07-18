#pragma once
#include "mymuduo/utils/Atomic.h"
#include "mymuduo/utils/Timestamp.h"
#include <functional>
using TimerCallback = std::function<void()>;

///
/// Internal class for timer event.
///
class Timer
{
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),// 过期时间
      interval_(interval),// 间隔
      repeat_(interval > 0.0),// 重复间隔
      sequence_(s_numCreated_.incrementAndGet())// 序号
  { }
  // 执行cb
  void run() const
  {
    callback_();
  }
  // 过期时间
  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }
  //
  int64_t sequence() const { return sequence_; }

  void restart(Timestamp now);
  // 获取序号
  static int64_t numCreated() { return s_numCreated_.get(); }

 private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
  const int64_t sequence_;

  static AtomicInt64 s_numCreated_;
};

