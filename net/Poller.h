#ifndef MYMUDUO_NET_POLLER_H
#define MYMUDUO_NET_POLLER_H

#include "noncopyable.h"
#include "Timestamp.h"
#include "Channel.h"

#include <vector>
#include <map>

namespace mymuduo{

    namespace net{
        
        class EventLoop;

        class Poller:noncopyable{
            public:
                using ChannelList = std::vector<Channel*>;
            public:
                Poller(EventLoop* loop);
                virtual ~Poller();

                //多路IO，可以是epoll或者poll继承重写
                virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) = 0;

                
                virtual void updateChannel(Channel* channel) = 0;
                virtual void removeChannel(Channel* channel) = 0;

                virtual bool hasChannel(Channel* channel) const ;

                //从语法上来看把这个函数在Poller.cc里面定义没问题，但是要知道的是
                //返回值是Epoll类型，也有可能是Poll类型的，这就导致需要引入这两个头文件
                //但是设计上又不建议基类把自类的头文件引入，所以就另开一个文件定义
                static Poller* newDefaultPoller(EventLoop* loop);
            protected:
                //第一个参数是fd
                using ChannelMap = std::map<int,Channel*>;
                ChannelMap channels_;
            
            private:
                EventLoop* ownerLoop_;
        };
    
    }   
}

#endif // !MYMUDUO_NET_POLLER_H