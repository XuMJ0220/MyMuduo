#ifndef MYMUDUO_BASE_THREAD_H
#define MYMUDUO_BASE_THREAD_H

#include "noncopyable.h"

#include <functional>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
namespace mymuduo{

    class Thread:noncopyable{

        public:
            using ThreadFunc = std::function<void()>;

            explicit Thread(ThreadFunc,const std::string& name = "");
            ~Thread();

            //启动线程
            void start();
            
            void join();
            //是否启动
            bool started() const { return started_;}
            //获取线程id
            pid_t tid() const { return tid_;}
            //获取线程的名字
            const std::string& name() const {return name_;}
            //获取线程的数量
            static int numCreated() {return numCreated_;}
        private:
            void setDefaultName();

            //线程是否已经启动
            bool started_;
            //线程是否要处于join
            bool joined_;
            //在这里不能直接就写成std::thread thread_
            //因为一旦这样子，线程立马就启动了
            //std::thread thread_;//不允许
            std::shared_ptr<std::thread> thread_;
            //线程id
            pid_t tid_;
            
            ThreadFunc func_;
            //线程的名字
            std::string name_;
            //创建了多少个线程
            static std::atomic_int numCreated_;
    };
}

#endif // !MYMUDUO_BASE_THREAD_H