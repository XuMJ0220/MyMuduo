#ifndef MYMUDUO_NET_POLLER_EPOLLPOLLER_H
#define MYMUDUO_NET_POLLER_EPOLLPOLLER_H

#include "Poller.h"
#include "Channel.h"

#include <vector>
#include <sys/epoll.h>

namespace mymuduo{

    namespace net{

        class EPollPoller:public Poller{
            
            public:
                //epoll_create在构造函数里实现
                EPollPoller(EventLoop* loop);
                ~EPollPoller() override;//加了override表示让编译器检查是否重写了基类的虚函数

                //epoll_wait在poll里实现
                Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;

                /*
                *   EventLoop----->ChannelList
                *            /  
                *            /
                *            ----->Poller: ChannelMap<fd,Channel*>     
                */

                //epoll_ctl在这个下面两个里实现，具体通过update实现
                //updateChannel和removeChanel主要是处理channels_
                //update才是具体对红黑树进行操作
                void updateChannel(Channel* channel) override;
                void removeChannel(Channel* channel) override;

            private:
                int epollfd_;//这是epoll_create返回的fd

                //EventList events_ 的初始化大小
                static const int kInitEventListSize = 16;
                using EventList = std::vector<epoll_event>;
                EventList events_;//EPollPoller中的events_数组存放的正是每次调用epoll_wait()后返回的就绪文件描述符(fd)对应的epoll_event结构体。

                void fillActiveChannels(int numEvents,
                                        ChannelList* activeChannels) const;
                
                void update(int operation,Channel* channel);
        };
    }
}

#endif // !MYMUDUO_NET_POLLER_EPOLLPOLLER_H