#ifndef MYMUDUO_NET_EVENTLOOPTHREAD_H
#define MYMUDUO_NET_EVENTLOOPTHREAD_H

#include "noncopyable.h"
#include "Thread.h"

#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>

#include "EventLoop.h"
namespace mymuduo{

    namespace net{

        class EventLoop;

        class EventLoopThread : noncopyable{
            public:
                using ThreadInitCallback = std::function<void(EventLoop*)>;
                
                EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),const std::string& name = std::string());
                ~EventLoopThread();
                
                EventLoop* startLoop();
            private:
                void threadFunc();

                //one loop per thread思想，一个loop_绑定一个thread_
                EventLoop* loop_;
                Thread thread_;
                //是否退出
                bool exiting_;

                std::mutex mutex_;
                std::condition_variable cond_;

                //回调
                ThreadInitCallback callback_;
        };
    }
}

#endif // !MYMUDUO_NET_EVENTLOOP_H
