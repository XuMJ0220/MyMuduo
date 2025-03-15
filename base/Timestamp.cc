#include "Timestamp.h"

#include <sstream>
#include <iomanip> //需要包含 <iomanip>（用于 std::put_time）
namespace mymuduo{

    //获得当前时间的Timestamp对象   
    Timestamp Timestamp::now(){
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        return Timestamp(now_time);
    }

    //转为“年-月-日 时:分:秒”的形式
    std::string Timestamp::toString() const{
        //转换为时间结构体(localtime)
        std::tm* local_time = std::localtime(&microSecondsSinceEpoch_);
        std::stringstream ss;
        ss<<std::put_time(local_time,"%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}
