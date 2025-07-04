#include "SocketsOps.h"
#include "Endian.h"
#include "Logger.h"
#include "Types.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

namespace mymuduo{

    namespace net{

        namespace sockets{
            // 将ip和port转换为sockaddr_in
            void fromIpPort(const char* ip,uint16_t port,sockaddr_in* addr){
                addr->sin_family = AF_INET;
                addr->sin_port = hostToNetwork16(port);
                if(::inet_pton(AF_INET,ip,&addr->sin_addr)<=0){//这里最前面的::是想要说优先用的是系统的inet_pton
                    std::string error_msg = "sockets::fromIpPort";
                    LOG_ERROR("%s",error_msg.c_str());
                }
            }

            void bindOrDie(int sockfd,const sockaddr_in& addr){
                int ret =  ::bind(sockfd,(sockaddr*)&addr,static_cast<socklen_t>(sizeof(sockaddr_in)));
                if(ret < 0){
                    LOG_FATAL("sockets::bindOrDie");
                }
            }

            void listenOrDie(int sockfd){
                int ret = ::listen(sockfd,1024);
                if(ret < 0){
                    LOG_FATAL("sockets::listenOrDie");
                }
            }

            //创建非阻塞的socket文件描述符,这里只用IPV4的
            //源码中参数是sa_family_t family，这里由于我们只用IPV4，就不添加了
            int createNonblockingOrDie(){
                //SOCK_NONBLOCK让socket非阻塞
                //SOCK_CLOEXEC在创建 socket 文件描述符时自动设置 close-on-exec 标志位。这意味着当进程执行 exec 系统调用时，该 socket 文件描述符会被自动关闭
                int sockfd = ::socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,IPPROTO_TCP);
                if(sockfd<0){
                    LOG_FATAL("%s:%s:%d sockets::createNonblockingOrDie",__FILE__,__FUNCTION__,__LINE__);
                }
                return sockfd;
            }

            //根据sockfd获得对应的ip和端口
            //这里只处理IPV4
            sockaddr_in getLocalAddr(int sockfd){
                sockaddr_in localaddr;
                bzero(&localaddr, sizeof(localaddr));
                socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));

                //getsockname()是根据传入的fd，然后返回根据这个fd的东西，给到了第二个参数和第三个参数传递出来
                if(::getsockname(sockfd,(sockaddr*)&localaddr,&addrlen) < 0){
                    LOG_ERROR("%s:%s:%d sockets::getLocalAddr",__FILE__,__FUNCTION__,__LINE__);
                }
                return localaddr;
            }

            // 写数据
            ssize_t write(int sockfd, const void* buf, size_t count)
            {
                return ::write(sockfd, buf, count);
            }
            
            // 获取socket错误
            int getSocketError(int sockfd)
            {
                int optval;
                socklen_t optlen = static_cast<socklen_t>(sizeof optval);
                
                if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
                {
                    return errno;
                }
                else
                {
                    return optval;
                }
            }
            
            // 关闭socket的写端
            void shutdownWrite(int sockfd)
            {
                if (::shutdown(sockfd, SHUT_WR) < 0)
                {
                    LOG_ERROR("sockets::shutdownWrite");
                }
            }
            
            // 从socket读取数据
            ssize_t read(int sockfd, void* buf, size_t count)
            {
                return ::read(sockfd, buf, count);
            }
        }
    }
}