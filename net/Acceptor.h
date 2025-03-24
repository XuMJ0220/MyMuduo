#ifndef MYMUDUO_NET_ACCEPTOR_H
#define MYMUDUO_NET_ACCEPTOR_H

#include "noncopyable.h"
#include "socket.h"
#include "Channel.h"

#include <functional>

namespace mymuduo{

    namespace net{

        class EventLoop;
        class InetAddress;

        class Acceptor:noncopyable{
            public:
                using NewConnectionCallback = std::function<void(int sockfd,const InetAddress& addr)>;
            
                Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport);
                ~Acceptor();

                void setNewConnectionCallback(const NewConnectionCallback& cb){
                    newCOnnectionCallback_ = cb;
                }

                void listen();

                bool listening() const {return listening_;}
            private:
                void handleRead();

                EventLoop* loop_;//这个是mainReactor的loop，也就是之前说的mainloop
                Socket acceptSocket_;
                Channel acceptChannel_;
                NewConnectionCallback newCOnnectionCallback_;
                bool listening_;

        };
    }
}

#endif // !MYMUDUO_NET_ACCEPTOR_H
