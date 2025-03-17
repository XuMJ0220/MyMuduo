#include "Poller.h"

namespace mymuduo{

    namespace net{

        Poller::Poller(EventLoop* loop)
            :ownerLoop_(loop)
        {
        }

        Poller::~Poller() = default;

        bool Poller::hasChannel(Channel* channel) const {
            auto it = channels_.find(channel->fd());
            //如果channel在channels确实存在，并且通过fd在channels找到的值和参数channel一样，则返回true
            return (it!=channels_.end()) && (it->second==channel);
        }
    }

}