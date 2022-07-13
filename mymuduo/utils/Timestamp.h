#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    // 返回现在时间
    static Timestamp now();
    // 判断是否合法
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    // 返回非法时间戳
    static Timestamp invalid()
    {
        return Timestamp();
    }
    // 将时间格式化为字符串
    std::string toString() const;
    // 交换时间
    void swap(Timestamp& that){
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

// 调整定时时间
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}