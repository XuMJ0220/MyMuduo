#include "TcpServer.h"
#include "Logging.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "SocketsOps.h"

namespace mymuduo{

    namespace net{

        //这里的listenAddr会被mainReactor用来创建监听套接字
        TcpServer::TcpServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& nameArg,
        Option option
        )
        :loop_(CheckNotNull<EventLoop>(loop)),
        ipPort_(listenAddr.toIpPort()),
        name_(nameArg),
        acceptor_(new Acceptor(loop,listenAddr,option == kNoReUsePort)),
        threadPool_(new EventLoopThreadPool(loop,name_)),
        connectionCallback_(defaultConnectionCallback),
        messageCallback_(defaultMessageCallback),
        nextConnId_(1)
        {

        }

        void TcpServer::newConnection(int sockfd,const InetAddress& peerAddr){
            EventLoop* ioLoop = threadPool_->getNextLoop();
            char buf[64];
            snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_);
            ++nextConnId_;
            std::string connName = name_+buf;

            LOG_INFO("TcpServer::newConnection [%s]- new connection [%s]",name_.c_str(),peerAddr.toIpPort().c_str());

            //创建一个localAddr
            InetAddress localAddr(socket::getLocalAddr(sockfd));

            //TcpConnectionPtr conn(new TcpConnection(ioLoop,
            //                                        connName,
            //                                        sockfd,
            //                                        localAddr
            //                                        peerAddr));

            //conn->setConnectionCallback
            //conn->setMessageCallback
            //conn->setWriteCompleteCallback
            //conn->setCloseCallback

            //ioLoop->runInLoop(std::bind(&TcpConnection::));
        }
    }
}
