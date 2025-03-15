#include "InetAddress.h"
#include "Types.h"
#include "Endian.h"
#include "SocketsOps.h"

//这里加上了static是因为想要变量只能够在这个文件使用
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

namespace mymuduo{

    namespace net{
        //这个构造函数如果第二个参数是flase,则表示是要0.0.0.0 如果是true则表示是要127.0.0.1
        InetAddress::InetAddress(uint16_t port ,bool loopbackOnly){
            //初始化一下addr_
            memZero(&addr_,sizeof(addr_));

            addr_.sin_family = AF_INET;
            in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
            addr_.sin_port = socket::hostToNetwork16(port);
            addr_.sin_addr.s_addr = socket::hostToNetwork32(ip);
        }

        InetAddress::InetAddress(mymuduo::StringArg ip,uint16_t port){
            memZero(&addr_,sizeof(addr_));
            // 将ip和port转换为sockaddr_in
            socket::fromIpPort(ip.c_str(),port,&addr_);
        }

        //获得ip字符串
        std::string InetAddress::toIp(){
            char buf[64] = {0};
            ::inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
            return buf;
        }
        //从网络的大端IP端口转为本地的小端端口IP
        std::string InetAddress::toIpPort(){
            //ip:port
            char buf[64] = {0};
            ::inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
            std::string ipport = std::string(buf)+":"+std::to_string(::ntohs(addr_.sin_port));
            return ipport;
        }
        //获得端口
        uint16_t InetAddress::port(){
            return ::ntohs(addr_.sin_port);
        }
    }
}