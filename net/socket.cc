#include "socket.h"
#include "SocketsOps.h"
#include "InetAddress.h"
#include "Types.h"
#include "Logging.h"

#include <unistd.h>
#include <netinet/tcp.h>

namespace mymuduo{

    namespace net{

        Socket::~Socket(){
            ::close(sockfd_);
        }

        //如果地址在使用就终止
        void Socket::bindAddress(const InetAddress& localaddr){
            socket::bindOrDie(sockfd_,localaddr.getSockAddr());
        }

        //如果地址在使用就终止
        void Socket::listen(){
            socket::listenOrDie(sockfd_);
        }

        //non-blocking非阻塞方式
        //错误返回-1
        //成功返回一个非负整数，且*peeraddr的内容被改变
        int Socket::accept(InetAddress* peeraddr){
            sockaddr_in addr;
            socklen_t len;
            memZero(&addr,sizeof(addr));

            int connfd = ::accept(sockfd_,(sockaddr*)&addr,&len);
            if(connfd >=0 ){
                peeraddr->setSockAddr(addr);
            }
            return connfd;
        }

        //关闭写端
        void Socket::shutdownWrite(){
            if(::shutdown(sockfd_,SHUT_WR)<0){
                LOG_ERROR("sockets::shutdownWrite");
            }
        }

        //下面几个是使能/失能
        //设置超时
        void Socket::setTcpNoDelay(bool on){
            int optval = on ? 1 : 0;
            ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,static_cast<socklen_t>(sizeof(optval)));
        }
        //地址复用，允许同一端口在关闭后被重新绑定
        void Socket::setReuseAddr(bool on){
            int optval = on ? 1 : 0;
            ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,static_cast<socklen_t>(sizeof(optval)));
        }
        
        void Socket::setReusePort(bool on){
            
        }
        //开启保活机制，定期检查连接是否存活
        void Socket::setKeepAlive(bool on){
            int optval = on ? 1 : 0;
            ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,&optval, static_cast<socklen_t>(sizeof optval));
        }
    }
}