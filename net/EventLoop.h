#ifndef MYMUDUO_NET_EVENTLOOP_H
#define MYMUDUO_NET_EVENTLOOP_H

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <vector>
#include <atomic>
#include <sys/types.h>
#include <memory>
#include <mutex>
#include <functional>

namespace mymuduo{

    namespace net{
        
        class Channel;
        class Poller;

        class EventLoop:noncopyable{
            public:
                using Functor = std::function<void()>;

            private:
                using ChannelList = std::vector<Channel*>;

                std::atomic_bool looping_;//事件循环运行标志
                std::atomic_bool quit_;//退出事件循环标志
                std::atomic_bool eventHandling_;//事件处理中标志
                std::atomic_bool callingPendingFunctors_;//正在执行等待中的回调
                const pid_t threadId_;//所属线程tid
                Timestamp pollReturnTime_;//返回发生事件的channels的时间点
                std::unique_ptr<Poller> poller_;//I/O复用对象(epoll/poll的抽象)
                
                int wakeupFd_;//主要作用：当mainloop获取一个新用户的channel，通过轮询算法选择一个subloop，以wakeupFd_d
                std::unique_ptr<Channel> wakeupChannel_;//专门用于wakeupFd的Channel(不暴露给用户)

                ChannelList activeChannels_;//本次poll返回的活动Channel列表
                Channel* currentActiveChannel_;//当前正在处理的Channel

                std::mutex mutex_;//互斥锁,用来保护下面的vector容器的线程安全操作
                std::vector<Functor> pendingFunctors_;//存储loop需要执行的所有回调操作

                void doPendingFunctors();//执行等待中的回调任务
                void handleRead();
            public:

                //每个线程只能有一个EventLoop实例
                EventLoop();
                ~EventLoop();

                //启动事件循环，必须在创建EventLoop的线程中调用
                //该函数会一直运行直到调用quit()
                void loop();

                //停止事件循环
                //注意：如果通过裸指针调用，不是100%安全的
                //建议通过shared_ptr<EventLoop>调用以确保线程安全
                void quit();
                
                //返回发生事件的channels的时间点
                Timestamp pollReturnTime() const 
                {
                    return pollReturnTime_;
                }

                //在事件循环线程中立即执行回调
                //如果在当前线程调用，则直接执行
                //线程安全，可从其他线程调用
                void runInLoop(Functor cb);
                //将回调加入事件循环的代执行队列
                //在完成事件轮询后执行
                //线程安全，可从其他线程调用
                void queueInLoop(Functor cb);

                void wakeup();
                //因为Channel和EPollPoller无法沟通，所以Chaneel的update
                //和remove调用EventLoop的updateChannel和removeChannel，
                //然后updateChannel和removeChannel去调用EPollPoller的
                //updateChannel和removeChannel
                void updateChannel(Channel* channel);
                void removeChannel(Channel* channel);
                bool hasChannel(Channel* channel);

                bool isInLoopThread() const
                {
                    return threadId_ == CurrentThread::tid();
                }

                // 断言当前线程是否是创建EventLoop的线程
                void assertInLoopThread()
                {
                    if (!isInLoopThread())
                    {
                        abortNotInLoopThread();
                    }
                }

                // 当断言失败时调用
                void abortNotInLoopThread();

                bool eventHandling() const
                {
                    return eventHandling_;
                }

                static EventLoop* getEventLoopOfCurrentThread();
        };
    }
}

#endif // !MYMUDUO_NET_EVENTLOOP_H
