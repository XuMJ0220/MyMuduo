#ifndef MYMUDUO_NET_CHANNEL_H
#define MYMUDUO_NET_CHANNEL_H

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

namespace mymuduo{

    namespace net{

        //前置声明，这样可以不包含头文件
        class EventLoop;

        //Channel理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN,EPOLLOUT事件
        //还绑定了Poller返回的具体事件
        class Channel:noncopyable{
            public:
                using EventCallback = std::function<void()>;
                using ReadEventCallback = std::function<void(Timestamp)>;

            private:
                //回调
                ReadEventCallback readCallback_;
                EventCallback writeCallback_;
                EventCallback closeCallback_;
                EventCallback errorCallback_;

            private:
                EventLoop* loop_;
                const int fd_;
                int events_;//注册fd感兴趣的事件类型
                int revents_;//poller返回的具体发生的事件
                int index_;

                std::weak_ptr<void> tie_;//避免循环引用
                bool tied_;
            
            private:
                //更新事件注册到EventLoop
                //最终通过Poller执行epool_ctl操作
                void update();
                //事件处理核心方法,根据revents_状态分发到对应的回调函数
                //处理POLLIN/POLLOUT/POLLERR等
                void handleEventWithGuard(Timestamp receiveTime);

                static const int kNoneEvent;
                static const int kReadEvent;
                static const int kWriteEvent;
            public:
                

                Channel(EventLoop* loop,int fd);
                ~Channel();

                //fd得到pooler通知以后,处理事件的函数
                void handleEvent(Timestamp receiveTIme);

                //设置回调函数
                //这里使用了move,是因为可以直接调用的function的移动赋值，而非拷贝赋值
                void setReadCallback(ReadEventCallback cb){ readCallback_ = std::move(cb);}
                void setWriteCallback(EventCallback cb){ writeCallback_ = std::move(cb);}
                void setCloseCallback(EventCallback cb){ closeCallback_ = std::move(cb);}
                void setErrorCallback(EventCallback cb){ errorCallback_ = std::move(cb);}

                //将Channel与其所有者对象（如 TcpConnection)的生命周期绑定，单把tie_提升为强智能指针可防止在事件处理过程中所有者对象被意外销毁
                void tie(const std::shared_ptr<void>&);

                int fd() const {return fd_;}
                int events() const {return events_;}
                void set_revents(int revet){revents_ = revet;}//pollers使用
                int revents() const {return revents_;}

                void enableReading(){ events_ |= kReadEvent; update(); }
                void disableReading(){ events_ &= ~kReadEvent; update(); }
                void enableWriting(){ events_ |= kWriteEvent; update(); }
                void disableWriting(){ events_ &= ~kWriteEvent; update();}
                void disablaAll(){ events_ = kNoneEvent; update();}

                bool isNodeEvent() const { return events_ == kNoneEvent;}
                bool isReading() const { return events_ == kReadEvent;}
                bool isWriting() const { return events_ == kWriteEvent;}

                //for Poller
                int index(){ return index_; }
                void set_index(int idx){ index_ = idx;}

                EventLoop* ownerLoop(){ return loop_;}
                //从EventLoop移除当前Channel的事件监听
                //实际调用EventLoop::removeChannel(),通过Poller执行epoll_ctl(EPOLL_CTL_DEL)
                void remove();
        };

    }

}

#endif // !MYMUDUO_NET_CHANNEL_H