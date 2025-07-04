#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include "Logging.h"

#include <unistd.h>

namespace mymuduo{

    namespace net{

        Acceptor::Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport)
            : loop_(loop),
            acceptSocket_(sockets::createNonblockingOrDie()),
            acceptChannel_(loop,acceptSocket_.fd()),
            listening_(false)
        {
            acceptSocket_.setReuseAddr(true);
            acceptSocket_.setReusePort(reuseport);
            acceptSocket_.bindAddress(listenAddr);
            //当我们用户使用了TcpServer::start()的时候，就会调用Acceptor.listen，如果有新用户连接，
            //就会执行一个回调（connfd打包成一个channel然后唤醒subloop，这个subloop后面负责监听这个channel）
            acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
        }

        Acceptor::~Acceptor()
        {
            acceptChannel_.disablaAll();
            acceptChannel_.remove();
        }

        void Acceptor::listen(){
            listening_ = true;
            acceptSocket_.listen();
            acceptChannel_.enableReading();
        }

        void Acceptor::handleRead(){
            InetAddress peerAddr;
            int connfd = acceptSocket_.accept(&peerAddr);
            if(connfd >= 0 ){
                if(newCOnnectionCallback_){
                    //轮询找到subloop,唤醒，分发到当前的新客户端的Channel
                    newCOnnectionCallback_(connfd,peerAddr);
                }else{
                    ::close(connfd);
                }
            }else{
                LOG_ERROR("%s:%s:%d accept err:%d",__FILE__,__FUNCTION__,__LINE__,errno);
                if(errno == EMFILE){
                    LOG_ERROR("%s:%s:%d sockfd reached limit err:%d",__FILE__,__FUNCTION__,__LINE__,errno);
                }
            }
        }
    }
}