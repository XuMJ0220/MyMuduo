#ifndef MYMUDUO_BASE_STRINGPIECE_H
#define MYMUDUO_BASE_STRINGPIECE_H

#include <string>

namespace mymuduo{
    //用于将C风格字符串传递给函数
    class StringArg
    {
        private:
            const char* str_;
        public:
            StringArg(const char* str):str_(str){}
            StringArg(const std::string& str):str_(str.c_str()){}
            const char* c_str() const{ return str_;}
    };
}

#endif // !MYMUDUO_BASE_STRINGPIECE_H
