#ifndef MYMUDUO_NET_INETADDRESS_H
#define MYMUDUO_NET_INETADDRESS_H

#include <netinet/in.h>
#include <sys/types.h>          
#include <sys/socket.h>

#include "copyable.h"
#include "StringPiece.h"

namespace mymuduo{

    namespace net{

        class InetAddress:public mymuduo::copyable{
            private:
                //原本muduo网络库使用了一个union存IPV4和IPV6的，这里就只用IPV4
                sockaddr_in addr_;
            public:
                //这个构造函数如果第二个参数是flase,则表示是要0.0.0.0 如果是true则表示是要127.0.0.1
                explicit InetAddress(uint16_t port = 0 ,bool loopbackOnly = false);
                InetAddress(mymuduo::StringArg ip,uint16_t port);
                explicit InetAddress(const sockaddr_in& addr)
                :addr_(addr)
                {}

                //获得family
                /// @return 
                sa_family_t family() const { return addr_.sin_family;}
                //获得ip字符串
                std::string toIp() const;
                //从网络的大端IP端口转为本地的小端端口IP
                std::string toIpPort() const;
                //获得端口
                uint16_t port() const;
                //获取socket的addr
                const sockaddr_in getSockAddr() const { return addr_;}
                //设置
                void setSockAddr(const sockaddr_in& addr) { addr_ = addr;}
        };

    }
}

#endif // !MYMUDUO_NET_INETADDRESS_H
