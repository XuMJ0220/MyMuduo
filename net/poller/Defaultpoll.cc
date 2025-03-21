#include "Poller.h"
#include "EPollPoller.h"
#include "PollPoller.h"

#include <cstdlib>
namespace mymuduo{

    namespace net{

        Poller* Poller::newDefaultPoller(EventLoop* loop){
            //如果环境变量有MUDUO_USE_POLL
            if(::getenv("MUDUO_USE_POLL")){
                return nullptr;
            }else{
                return new EPollPoller(loop);
            }
        }
    }

}