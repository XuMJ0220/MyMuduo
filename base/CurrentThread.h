#ifndef MYMUDUO_NET_CURRENTTHREAD_H
#define MYMUDUO_NET_CURRENTTHREAD_H

namespace mymuduo{

    namespace CurrentThread{
        //这里只有家了extern才表示是申明，不然就是定义，在.h文件里是不能定义的
        //__thread是线程局部存储，每个线程都有一份独立实体
        extern __thread int t_cachedTid;//缓存Linux线程id，通过syscall(SYS_gettid)获得线程的真实线程id 
        extern __thread char t_tidString[32];//缓存线程ID的字符串表示形式
        extern __thread int t_tidStringLength;//缓存线程ID字符串的长度
        extern __thread const char* t_threadName;//缓存线程名称
        void cacheTid();
        
        //获取当前线程tid
        inline int tid(){
            //如果t_cachedTid==0，那么返回结果为真
            //__builtin_expect(t_cachedTid == 0, 0)是让编译器去优化
            //告诉编译器t_cachedTid == 0这个条件很少发生
            //如果是__builtin_expect(t_cachedTid == 0, 1).则告诉编译器t_cachedTid == 0这个条件经常发生
            if(__builtin_expect(t_cachedTid == 0,0)){
                cacheTid();
            }
            return t_cachedTid;
        }
    }
}

#endif // !MYMUDUO_NET_CURRENTTHREAD_H