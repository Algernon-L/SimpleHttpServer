#include "mymuduo/net/Timer.h"

AtomicInt64 Timer::s_numCreated_;

// 重启定时器
// 1.重复 更新时间
// 2.不重复 定义为非法时间
void Timer::restart(Timestamp now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = Timestamp::invalid();
  }
}
