#include "EventLoop.h"
#include "Poller.h"
#include "EPollPoller.h"
#include "Logging.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <functional>
#include <unistd.h>

namespace mymuduo{
    //每个线程只能有一个EventLoop,t_loopInThisThread指向当前的EventLoop
    __thread mymuduo::net::EventLoop* t_loopInThisThread = nullptr;

    const int kPollTimeMs = 10000;

    int createEventFd(){
        int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
        if(evtfd < 0){
            LOG_ERROR("Failed in eventfd");     
            abort();
        }
        return evtfd;
    }
}

namespace mymuduo{

    namespace net{

        EventLoop* EventLoop::getEventLoopOfCurrentThread(){
            return t_loopInThisThread;
        }

        EventLoop::EventLoop()
        :looping_(false),
        quit_(true),
        eventHandling_(false),
        callingPendingFunctors_(false),
        threadId_(CurrentThread::tid()),
        poller_(Poller::newDefaultPoller(this)),
        wakeupFd_(createEventFd()),
        wakeupChannel_(new Channel(this,wakeupFd_)),
        currentActiveChannel_(nullptr)
        {
            LOG_INFO("EventLoop created %p in thread %d",this,(int)threadId_);
            if(t_loopInThisThread){
                LOG_FATAL("%d thread has existed EventLoop %p",(int)threadId_,t_loopInThisThread);
            }else{
                t_loopInThisThread = this;
            }
            //要把需要唤醒的channel的读功能打开，我才能发信息过去
            wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
            wakeupChannel_->enableReading();
        }

        EventLoop::~EventLoop(){
            LOG_INFO("EventLoop %p of thread %d destructs in thread %d",this,threadId_,CurrentThread::tid());
            wakeupChannel_->disablaAll();
            wakeupChannel_->remove();
            ::close(wakeupFd_);
            t_loopInThisThread = nullptr;
        }

        //启动事件循环，必须在创建EventLoop的线程中调用
        //该函数会一直运行直到调用quit()
        void EventLoop::loop(){

            looping_ = true;
            quit_ = false;
            LOG_INFO("EventLoop %p start looping",this);

            while(!quit_){
                activeChannels_.clear();
                pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);

                eventHandling_ = true;
                for(Channel* channel:activeChannels_){
                    //这里的channel可能是 wakeFd_的读事件 其他客户端发来的请求
                    //wakeFd_的读事件执行的回调就是EventLoop::handleRead
                    channel->handleEvent(pollReturnTime_);
                }
                eventHandling_ = false;
                //doPendingFnctors()是执行当前EventLoop事件循环需要处理的回调操作
                /*
                * 比如mainLoop是用来处理客户端连接是的fd，将fd打包，分发到subLoop中
                *mainLoop需要先注册一个回调cb(需要subLoop来执行)，这个cb可能是客户连进来的fd插入到epool树
                *经过wakeup后，pooler->poll会通过阻塞，之后就可以执行mainLoop注册的cb
                */
                doPendingFunctors();

                LOG_INFO("EventLoop %p stop looping",this);
                looping_ = false;
            }
        }

        //停止事件循环
        //注意：如果通过裸指针调用，不是100%安全的
        //建议通过shared_ptr<EventLoop>调用以确保线程安全
        void EventLoop::quit(){
            quit_ = true;

            if(!isInLoopThread()){//如果当前的EventLoop的线程id与当前线程部匹配，
                                //需要唤醒，不然可能在自己的线程中卡在了loop()的poller->pool()
               wakeup(); 
            }
        }

        void EventLoop::handleRead(){
            uint64_t one = 1;
            ssize_t n = ::read(wakeupFd_,&one,sizeof(one));
            if(n!=sizeof(one)){
                LOG_ERROR("EventLoop::handleRead() reads %l bytes instead of 8",n);
            }
        }

        void EventLoop::wakeup(){
            uint64_t one = 1;
            ssize_t n = ::write(wakeupFd_,&one,sizeof(one));
            if(n!=sizeof(one)){
                LOG_ERROR("EventLoop::wakeup() write %l bytes instead of 8",n);
            }
        }

        //在事件循环线程中立即执行回调
        //如果在当前线程调用，则直接执行
        //线程安全，可从其他线程调用
        void EventLoop::runInLoop(Functor cb){
            if(isInLoopThread()){
                cb();
            }else{
                queueInLoop(std::move(cb));
            }
        }

        //将回调加入事件循环的代执行队列
        //在完成事件轮询后执行
        //线程安全，可从其他线程调用
        void EventLoop::queueInLoop(Functor cb){
            {
                std::unique_lock<std::mutex> lock(mutex_);
                pendingFunctors_.push_back(std::move(cb));
            }
            
            //callingPendingFunctors_表示正在执行回调(loop()中的doPendingFunctors（）)，
            //当回调执行完了之后，又循环回去阻塞在了poller->pool，所以要唤醒
            if(!isInLoopThread()||callingPendingFunctors_){
                wakeup();
            }
        }

        //因为Channel和EPollPoller无法沟通，所以Chaneel的update
        //和remove调用EventLoop的updateChannel和removeChannel，
        //然后updateChannel和removeChannel去调用EPollPoller的
        //updateChannel和removeChannel
        void EventLoop::updateChannel(Channel* channel){
            poller_->updateChannel(channel);
        }
        void EventLoop::removeChannel(Channel* channel){
            poller_->removeChannel(channel);
        }

        bool EventLoop::hasChannel(Channel* channel){
            return poller_->hasChannel(channel);
        }

        void EventLoop::doPendingFunctors(){
            std::vector<Functor> functors;
            callingPendingFunctors_ = true;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                functors.swap(pendingFunctors_);//这里进行了交换，pendingFunctors_就变成空了
                                                //这样在queInLoop（）就又可以往这里面赛东西了，
                                                //如果不交换，在没有执行完回调的时候，是不能有其他回调
                                                //塞进pendingFunctors_的
            }

            for(auto& Functor:functors){
                Functor();
            }

            callingPendingFunctors_ = false;
        }//执行等待中的回调任务
    }
}