#include "Thread.h"
#include "CurrentThread.h"

#include <sys/types.h> //pid_t
#include <unistd.h> //syscall
#include <sys/syscall.h> //SYS_gettid
#include <stdio.h>
#include <semaphore.h>

namespace mymuduo{

    namespace detail{

        pid_t gettid(){
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }

    namespace CurrentThread{

        void cacheTid(){
            if(t_cachedTid == 0){
                t_cachedTid = detail::gettid();
                t_tidStringLength = snprintf(t_tidString,sizeof(t_tidString),"%5d",t_cachedTid);
            }
        }
    }

    //numCreated_是类静态成员变量
    std::atomic_int Thread::numCreated_ = ATOMIC_VAR_INIT(0);
    Thread::Thread(ThreadFunc func,const std::string& name)
        :started_(false),
        joined_(false),
        tid_(0),
        func_(std::move(func)),
        name_(name)
    {
        setDefaultName();
    }

    Thread::~Thread()
    {
        if(started_&&!joined_){
            thread_->detach();//thread类提供的设置分离线程的方法
        }
    }

    void Thread::setDefaultName(){
        int num = ++numCreated_;
        if(name_.empty()){
            char buf[32] = {0};
            snprintf(buf,sizeof(buf),"Thread%d",num);//默认名字设置成Thread几
        }
    }

    //启动线程
    void Thread::start(){
        started_ = true;
        sem_t sem;
        sem_init(&sem,0,0);

        //开启线程
        thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
            //获取线程id
            tid_ = CurrentThread::tid();
            sem_post(&sem);
            //执行专门线程函数
            func_();
        }));

        //在这里必须等待获取到了上面的线程id才能继续往下执行
        sem_wait(&sem);
    }

    void Thread::join(){
        joined_ = true;
        thread_->join();
    }
}