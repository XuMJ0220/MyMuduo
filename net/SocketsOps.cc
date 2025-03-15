#include "SocketsOps.h"
#include "Endian.h"
#include "Logging.h"

namespace mymuduo{

    namespace net{

        namespace socket{
            // 将ip和port转换为sockaddr_in
            void fromIpPort(const char* ip,uint16_t port,sockaddr_in* addr){
                addr->sin_family = AF_INET;
                addr->sin_port = hostToNetwork16(port);
                if(::inet_pton(AF_INET,ip,&addr->sin_addr)<=0){//这里最前面的::是想要说优先用的是系统的inet_pton
                    std::string error_msg = "sockets::fromIpPort";
                    LOG_ERROR("%s",error_msg.c_str());
                }
            }

        }
    }
}