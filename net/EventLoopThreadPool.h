#ifndef MYMUDUO_NET_EVENTLOOPTHREADPOOL
#define MYMUDUO_NET_EVENTLOOPTHREADPOOL

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>
namespace mymuduo{

    namespace net{

        class EventLoopThreadPool : noncopyable{

            public:
                using ThreadInitCallback = std::function<void(EventLoop*)>;

                EventLoopThreadPool(EventLoop* baseloop,const std::string& nameArg);
                ~EventLoopThreadPool();

                void setThreadNum(int numThreads){ numThreads_ = numThreads; }
                void start(const ThreadInitCallback& cb = ThreadInitCallback());

                //如果工作在线程中，baseloop_默认以轮询的方式分配channel给subloop
                EventLoop* getNextLoop();
                std::vector<EventLoop*> getAllLoops();

                bool started() const { return started_;}
                const std::string& name() const { return name_;}

            private:
                EventLoop* baseloop_;//这里的baseloop_就是前面一直说的mainloop
                std::string name_;
                bool started_;
                int numThreads_;//subloop所在的EventLoopThread的数量
                int next_;
                std::vector<std::unique_ptr<EventLoopThread>> threads_;
                std::vector<EventLoop*> loops_;
        };
    }
}

#endif // !MYMUDUO_NET_EVENTLOOPTHREADPOOL
