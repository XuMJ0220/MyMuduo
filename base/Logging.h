#ifndef MYMUDUO_BASE_LOGGING_H
#define MYMUDUO_BASE_LOGGING_H

#include <string>

#include "noncopyable.h"

namespace mymuduo{
    //LOG_INFO("%s %d",arg1,arg2);
    #define LOG_INFO(logarg,...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(Logger::LogLevel::INFO);\
        char buf[1024];\
        snprintf(buf,1024,logarg,##__VA_ARGS__);\
        logger.log(buf);\
    }\
    while(0);

    #define LOG_ERROR(logarg,...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(Logger::LogLevel::ERROR);\
        char buf[1024];\
        snprintf(buf,1024,logarg,##__VA_ARGS__);\
        logger.log(buf);\
    }\
    while(0);

    #define LOG_FATAL(logarg,...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(Logger::LogLevel::FATAL);\
        char buf[1024];\
        snprintf(buf,1024,logarg,##__VA_ARGS__);\
        logger.log(buf);\
    }\
    while(0);

//#define MUDEBUG
#ifdef MUDEBUG
    #define DEBUG_INFO(logarg,...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(Logger::LogLevel::DEBUG);\
        char buf[1024];\
        snprintf(buf,1024,loarg,##__VA_ARGS__);\
        logger.log(buf);\
    }\
    while(0);
#else
     #define DEBUG_INFO(logarg,...)
#endif
    //输出一个日志类，设计类单例模式
    class Logger:noncopyable{
        private:
            
            int logLevel_;
            Logger(){}
        public:
            enum LogLevel{
                    INFO,//普通信息
                    ERROR,//错误信息
                    FATAL,//core错误信息
                    DEBUG,//调试信息
                };
            //获取日志的唯一实例化对象
            static Logger& instance();    
            //设置日志级别
            void setLogLevel(int level);
            //写日志
            void log(std::string msg);
    };
}

#endif