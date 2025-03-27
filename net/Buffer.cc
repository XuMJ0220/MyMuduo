#include "Buffer.h"

#include <sys/uio.h>
#include <errno.h>
namespace mymuduo{

    namespace net{

        //从fd上读取数据
        ssize_t Buffer::readFd(int fd,int* saveErrno){
            
            char extrabuf[65536];
            struct iovec vec[2];

            const size_t writable = writableBytes();
            vec[0].iov_base = begin()+writeIndex_;
            vec[0].iov_len = writable;
            vec[1].iov_base = extrabuf;
            vec[1].iov_len = sizeof(extrabuf);

            const int iovcnt = (writable<sizeof(extrabuf))?2:1;
            const ssize_t n = ::readv(fd,vec,iovcnt);

            if(n<0){
                *saveErrno = errno;
            }else if((size_t)n<=writable){
                writeIndex_+=n;
            }else{
                writeIndex_ = buffer_.size();
                //此时buffer_已经满了，先把一些东西村到extrabuf，然后通过append扩容
                append(extrabuf,n-writable);
            }
            return n;
        }
    }
}