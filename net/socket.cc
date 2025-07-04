#include "socket.h"
#include "SocketsOps.h"
#include "InetAddress.h"
#include "Types.h"
#include "Logger.h"

#include <unistd.h>
#include <netinet/tcp.h>
#include <string.h>

namespace mymuduo{

    namespace net{

        Socket::~Socket(){
            ::close(sockfd_);
        }

        void Socket::bindAddress(const InetAddress& localaddr){
            sockets::bindOrDie(sockfd_,localaddr.getSockAddr());
        }

        void Socket::listen(){
            sockets::listenOrDie(sockfd_);
        }

        //non-blocking非阻塞方式
        //错误返回-1
        //成功返回一个非负整数，且*peeraddr的内容被改变
        int Socket::accept(InetAddress* peeraddr){
            sockaddr_in addr;
            socklen_t len = static_cast<socklen_t>(sizeof(addr));
            bzero(&addr, sizeof(addr));

            int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
            if(connfd >= 0){
                peeraddr->setSockAddr(addr);
            }
            return connfd;
        }

        //关闭写端
        void Socket::shutdownWrite(){
            sockets::shutdownWrite(sockfd_);
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
            int optval = on ? 1 : 0;
            #ifdef SO_REUSEPORT
            ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
            #else
            if (on)
            {
                LOG_ERROR("SO_REUSEPORT is not supported.");
            }
            #endif
        }
        //开启保活机制，定期检查连接是否存活
        void Socket::setKeepAlive(bool on){
            int optval = on ? 1 : 0;
            ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,&optval, static_cast<socklen_t>(sizeof optval));
        }
    }
}