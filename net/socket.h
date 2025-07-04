#ifndef MYMUDUO_NET_SOCKET_H
#define MYMUDUO_NET_SOCKET_H

#include "noncopyable.h"

namespace mymuduo{

    namespace net{

        class InetAddress;

        class Socket:noncopyable{

            public:
                explicit Socket(int sockfd):sockfd_(sockfd){}
                ~Socket();

                int fd() const { return sockfd_;}

                
                void bindAddress(const InetAddress& localaddr);
                
                void listen();

                //non-blocking非阻塞方式
                //错误返回-1
                //成功返回一个非负整数，且*peeraddr的内容被改变
                int accept(InetAddress* peeraddr);
                
                //关闭写端
                void shutdownWrite();

                //下面几个是使能/失能
                //设置超时
                void setTcpNoDelay(bool on);
                //地址复用，允许同一端口在关闭后被重新绑定
                void setReuseAddr(bool on);
                void setReusePort(bool on);
                //开启保活机制，定期检查连接是否存活
                void setKeepAlive(bool on);

            private:
                const int sockfd_;
        };
    }
}

#endif // !MYMUDUO_NET_SOCKET_H
