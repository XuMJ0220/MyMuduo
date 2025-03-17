#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

#include <sys/epoll.h>
#include <poll.h>

namespace mymuduo{

    namespace net{

        const int Channel::kNoneEvent = 0;
        const int Channel::kReadEvent = POLLIN | POLLPRI;
        const int Channel::kWriteEvent = POLLOUT;

        Channel::Channel(EventLoop* loop,int fd)
        :loop_(loop),
        fd_(fd),
        events_(kNoneEvent),
        revents_(kNoneEvent),
        index_(-1),
        tied_(false)
        {}

        Channel::~Channel()
        {}

        //将Channel与其所有者对象（如 TcpConnection)的生命周期绑定，单把tie_提升为强智能指针可防止在事件处理过程中所有者对象被意外销毁
        void Channel::tie(const std::shared_ptr<void>& obj){
            tie_ = obj;
            tied_ = true;
        }

        //更新事件注册到EventLoop
        //最终通过Poller执行epool_ctl操作
        void Channel::update(){
            
            //loop_->updateChannel(this);
        }

        //为了从EventLoop移除当前Channel的事件监听
        //实际调用EventLoop::removeChannel(),通过Poller执行epoll_ctl(EPOLL_CTL_DEL)
        void Channel::move(){
        
            //loop_->removeChannel(this);
        }

        //fd得到pooler通知以后,处理事件的函数
        void Channel::handleEvent(Timestamp receiveTIme){
            std::shared_ptr<void> guard;
            if(tied_){
                guard = tie_.lock();//把tie_提升为强智能指针，结果给到guard
                if(guard){//如果提升成功，guard将不是nullptr
                    handleEventWithGuard(receiveTIme);
                }
            }else{
                handleEventWithGuard(receiveTIme);
            }
        }

        //事件处理核心方法,根据revents_状态分发到对应的回调函数
        //处理POLLIN/POLLOUT/POLLERR等
        void Channel::handleEventWithGuard(Timestamp receiveTime){
            LOG_INFO("channel handleEvent revents : %d",revents_);
            //POLLHUP会在连接挂起 TCP对端关闭 管道写端关闭触发
            if((revents_& POLLHUP) && !(revents_&POLLIN)){
                if(closeCallback_){
                    //LOG_INFO("fd = %d Channel::handle_event() POLLHUP",fd_);
                    closeCallback_();
                }
            }
            //- 文件描述符未打开 当 poll 监视的 fd 从未被打开，或已被关闭时触发
            //- 非法操作 例如试图 poll 一个普通文件（非socket/pipe等）
            //- 内核检测到矛盾 fd 的状态与请求的事件不兼容（如监视写事件但 fd 是只读的）
            if(revents_&POLLNVAL){
                LOG_INFO("fd = %d Channel::handle_event() POLLNVAL",fd_);
            }

            if(revents_&(POLLNVAL|POLLERR)){
                if(errorCallback_){
                    errorCallback_();
                }
            }

            if(revents_&(POLLIN|POLLPRI|POLLRDHUP)){
                if(readCallback_){
                    readCallback_(receiveTime);
                }
            }

            if(revents_&POLLOUT){
                if(writeCallback_){
                    writeCallback_();
                }
            }
        }
    }
}