#ifndef MYMUDUO_NET_ENDIAN_H
#define MYMUDUO_NET_ENDIAN_H

#include <stdint.h>
#include <endian.h>

namespace mymuduo{

    namespace net{

        namespace socket{
            //把本地16位的主机小端字节序转转为网络16位的大端字节序
            inline uint16_t hostToNetwork16(uint16_t host16){
                //h:host
                //to:转为
                //be:big-endian 大端序(网络字节序)
                //32:32位数据
                return htobe16(host16);
            }

            //把本地32位的主机小端字节序转转为网络32位的大端字节序
            inline uint32_t hostToNetwork32(uint32_t host32){
                //h:host
                //to:转为
                //be:big-endian 大端序(网络字节序)
                //32:32位数据
                return htobe32(host32);
            }

            
        }
    }
    
}

#endif // !MYMUDUO_BASE_ENDIAN_H