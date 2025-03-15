//这是用来对socket的一些操作
#ifndef MYMUDUO_NET_SOCKETOPS_H
#define MYMUDUO_NET_SOCKETOPS_H
#include <arpa/inet.h>
namespace mymuduo{

    namespace net{

        namespace socket{
            // 将ip和port转换为sockaddr_in
            void fromIpPort(const char* ip,uint16_t port,sockaddr_in* addr);

        }

    }

}

#endif // !MYMUDUO_NET_SOCKETOPS_H#define
