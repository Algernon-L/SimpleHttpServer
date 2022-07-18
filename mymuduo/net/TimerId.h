#pragma once
#include "mymuduo/net/Timer.h"
// 绑定序号和定时器
class Timer;
///
/// An opaque identifier, for canceling Timer.
///
class TimerId
{
 public:
  TimerId()
    : timer_(NULL),
      sequence_(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
  {
  }

  // default copy-ctor, dtor and assignment are okay

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};
#pragma once