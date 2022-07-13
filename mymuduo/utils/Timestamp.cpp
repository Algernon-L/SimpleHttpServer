#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <time.h>
#include "mymuduo/utils/Timestamp.h"

// 时间戳

Timestamp::Timestamp() : microSecondsSinceEpoch_(0)
{
}
// 指定时间戳
Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}
// 返回现在的时间戳
Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}
// 格式化时间戳
std::string Timestamp::toString() const
{
    char buf[128] = {0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}
#endif
// #include <iostream>
// int main() {
//     std::cout << Timestamp::now().toString() << std::endl;
//     return 0;
// }
