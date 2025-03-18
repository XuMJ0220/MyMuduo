#include "EPollPoller.h"
#include "Logging.h"
#include "Types.h"

#include <errno.h>
#include <unistd.h>

namespace {
    //该channel还没有被添加channels_
    const int kNew = -1;
    //该channel已经添加进了channels_
    const int kAdded = 1;
    //该channel从channels_里面被移除
    const int kDelete = 2;
}
namespace mymuduo{

    namespace net{

        EPollPoller::EPollPoller(EventLoop* loop)
            :Poller(loop),
            epollfd_(epoll_create1(EPOLL_CLOEXEC)),
            events_(kInitEventListSize)
        {
            //epoll_create失败
            if(epollfd_<0){
                LOG_FATAL("epoll_create1 error: %d \n",errno);
            }
        }

        EPollPoller::~EPollPoller()
        {
           ::close(epollfd_);
        }

        /*
        *   EventLoop----->ChannelList
        *            /  
        *            /
        *            ----->Poller: ChannelMap<fd,Channel*>     
        */       
        void EPollPoller::updateChannel(Channel* channel){
            const int index = channel->index();

            LOG_INFO("fd = %d events = %d index = %d"
            ,channel->fd(),channel->events(),channel->index());

            //index在Channel创建的时候初始化为-1，此时还没有被添加进channels_
            //channels_在Poller这个类里面，EPollPoller继承自Poller，自然也有channels_

            //该channel还没有被添加channels_或该channel从channels_里面被移除
            if( index == kNew || index == kDelete){
                int fd = channel->fd();

                if(index == kNew){//如果是没有被添加进channels_，那么就添加进来
                    channels_[fd] = channel;
                }

                channel->set_index(kAdded);
                update(EPOLL_CTL_ADD,channel);
            }else{//channel已经在channels里面了
                int fd = channel->fd();
                if(channel->isNodeEvent()){//如果该fd不监听任何事件
                    //删除
                    update(EPOLL_CTL_DEL,channel);
                }else{
                    update(EPOLL_CTL_MOD,channel);
                }
            }
        }

        void EPollPoller::removeChannel(Channel* channel){
            int fd = channel->fd();
            channels_.erase(fd);

            int index = channel->index();
            if(index == kAdded){
                update(EPOLL_CTL_DEL,channel);
            }
            channel->set_index(kNew);  
        }

        void EPollPoller::update(int operation,Channel* channel){
            epoll_event event;
            memZero(&event,sizeof(event));
            event.events = operation;
            event.data.ptr = channel;
            int fd = channel->fd();

            if(::epoll_ctl(epollfd_,operation,fd,&event)<0){//如果失败了
                //如果是删除失败
                if(operation == EPOLL_CTL_DEL){
                    LOG_ERROR("epoll_ctl del error : %d",errno);
                }else{
                    LOG_FATAL("epoll_ctl add/mod error : %d",errno);
                }
            }
        }

        //epoll_wait在poll里实现
        //一个线程就只有一个EventLoop，一个EvevtLoop就一个loop
        Timestamp EPollPoller::poll(int timeoutMs,ChannelList* activeChannels){
            LOG_INFO("func = %s => fd total num: %lu",__FUNCTION__,channels_.size());
            
            //注意这里第二个参数传入时候的技巧可以学一学
            int numEvents = epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
        
            int savedErrno = errno;
            Timestamp now(Timestamp::now());

            if(numEvents > 0){
                LOG_INFO("%d events happened",numEvents);

                //将内核返回的epoll_event转换为活跃Channel列表，通过Channel::set_revents设置事件类型
                fillActiveChannels(numEvents,activeChannels);
                if(size_t(numEvents) == events_.size()){
                    events_.resize(events_.size()*2);
                }
            }
            else if(numEvents == 0){
                LOG_INFO("nothing happened");
            }
            else{
                if(savedErrno!=EINTR){
                    errno = savedErrno;
                    LOG_ERROR("EPollerPoller::poll()");
                }
            }
            return now;
        }

        void EPollPoller::fillActiveChannels(int numEvents,
                                ChannelList* activeChannels) const
        {
            for(int i =0 ;i < numEvents; i++){
                //现在的events_里都是都是需要响应的事件
                Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
                channel->set_revents(events_[i].events);
                activeChannels->push_back(channel);
            }
        }
    }
}