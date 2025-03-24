//这是用来对socket的一些操作
#ifndef MYMUDUO_NET_SOCKETOPS_H
#define MYMUDUO_NET_SOCKETOPS_H
#include <arpa/inet.h>
namespace mymuduo{

    namespace net{

        namespace socket{
            // 将ip和port转换为sockaddr_in
            void fromIpPort(const char* ip,uint16_t port,sockaddr_in* addr);
            
            //创建非阻塞的socket文件描述符,这里只用IPV4的
            //源码中参数是sa_family_t family，这里由于我们只用IPV4，就不添加了
            int createNonblockingOrDie();

            void bindOrDie(int sockfd,const sockaddr_in& addr);
            void listenOrDie(int sockfd);
            
        }

    }

}

#endif // !MYMUDUO_NET_SOCKETOPS_H#define
