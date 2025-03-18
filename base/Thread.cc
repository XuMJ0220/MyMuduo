#include "Thread.h"
#include "CurrentThread.h"

#include <sys/types.h> //pid_t
#include <unistd.h> //syscall
#include <sys/syscall.h> //SYS_gettid
#include <stdio.h>

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
}