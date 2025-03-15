#include "Logging.h"

#include <iostream>

namespace mymuduo{

    //获取日志的唯一实例化对象
    Logger& Logger::instance(){
        static Logger logger;
        return logger;
    }    

    //设置日志级别
    void Logger::setLogLevel(int level){
        logLevel_ = level;
    }

    //写日志
    void Logger::log(std::string msg){
        switch (logLevel_)
        {
        case Logger::LogLevel::INFO :
            std::cout<<"[INFO]";
            break;
        case Logger::LogLevel::ERROR :
            std::cout<<"[ERROR]";
            break;
        case Logger::LogLevel::FATAL :
            std::cout<<"[FATAL]";
            break;
        case Logger::LogLevel::DEBUG :
            std::cout<<"[DEBUG]";
            break;    
        default:
            break;
        }
        std::cout<<"print time"<<" : "<<msg<<std::endl;
    }    
}