#ifndef MYMUDUO_BEASE_TIMESTAMP_H
#define MYMUDUO_BEASE_TIMESTAMP_H

#include <ctime>
#include <chrono>
#include <string>
namespace mymuduo{
    //使用例子：获取当前时间
    //std::string now_time = Timestamp::now().to_string();

    class Timestamp{
        private:
            std::time_t microSecondsSinceEpoch_;
        public:
            Timestamp():microSecondsSinceEpoch_(0)
            {}
            explicit Timestamp(int64_t microSecondsSinceEpochArg)
            :microSecondsSinceEpoch_(microSecondsSinceEpochArg)
            {}

            //获得当前时间的Timestamp对象
            static Timestamp now();
            //转为“年-月-日 时:分:秒”的形式
            std::string toString() const;
    };
    
}

#endif // !MYMUDUO_BEASE_TIMESTAMP_H