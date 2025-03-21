#include "EventLoopThread.h"

#include <memory>
namespace mymuduo{

    namespace net{

        EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,const std::string& name)
            :loop_(nullptr),
            exiting_(false),
            thread_(std::bind(&EventLoopThread::threadFunc,this),name),
            mutex_(),
            cond_(),
            callback_(cb)
        {
        }

        EventLoopThread::~EventLoopThread()
        {
            exiting_ = true;
            if(loop_!=nullptr){
                loop_->quit();
                thread_.join();
            }
        }

        EventLoop* EventLoopThread::startLoop(){

            thread_.start();//启动底层的新线程，EventLoopThread::threadFunc在构造的时候就传进去了
        
            //需要等到threadFunc()的loop_ = &loop执行完
            EventLoop* loop = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while(loop==nullptr){
                    cond_.wait(lock);
                }
                loop = loop_;
            }
            return loop;//返回启动了的loop
        }

        void EventLoopThread::threadFunc(){
            //创建一个EventLoop
            EventLoop loop;

            if(callback_){
                callback_(&loop);
            }

            {
                std::unique_lock<std::mutex> lock(mutex_);
                loop_ = &loop;//把loop给到loop_
                cond_.notify_one();
            }

            loop_->loop();
            std::unique_lock<std::mutex> lock();
            loop_ = nullptr;
        }
    }
}