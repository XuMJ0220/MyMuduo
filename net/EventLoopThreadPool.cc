#include "EventLoopThreadPool.h"

namespace mymuduo{

    namespace net{

        EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop,const std::string& nameArg)
            :baseloop_(baseloop),
            name_(nameArg),
            started_(false),
            numThreads_(0),
            next_(0)
        {
        }

        EventLoopThreadPool::~EventLoopThreadPool()
        {
            //不需要删除loop，因为那是一个静态变量
        }

        void EventLoopThreadPool::start(const ThreadInitCallback& cb){
            started_ = true;

            for(int i=0;i<numThreads_;i++){//如果说numTHreads为0是进不来这里的
                //名字
                char buf[name_.size()+32];
                snprintf(buf,sizeof(buf),"%s%d",name_.c_str(),i);
                EventLoopThread* t = new EventLoopThread(cb,buf);
                threads_.push_back(std::unique_ptr<EventLoopThread>(t));
                loops_.push_back(t->startLoop());
            }

            if(numThreads_==0&&cb){//如果只有一个线程的话，那么一定是mainloop
                cb(baseloop_);
            }
        }

        std::vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
            if(loops_.empty()){
                return std::vector<EventLoop*>(1,baseloop_);//1个baseloop_
            }else{
                return loops_;
            }
        }
                
        //如果工作在线程中，baseloop_默认以轮询的方式分配channel给subloop
        EventLoop* EventLoopThreadPool::getNextLoop(){
            EventLoop* loop = nullptr;

            if(!loops_.empty()){
                loop = loops_[next_];
                next_++;
                if(next_>=loops_.size()){
                    next_=0;
                }
            }
            return loop;
        }
    }
}