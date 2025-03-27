#ifndef MYMUDUO_NET_BUFFER_H
#define MYMUDUO_NET_BUFFER_H

#include "copyable.h"

#include <vector>
#include <algorithm>
#include <string>

namespace mymuduo{

    namespace net{

        //缓冲区：
        /*
        +--------------------+-----------------+------------------+
        |prependatable bytes | readable bytes  | writable bytes   |
        |                    |    (content)    |                  |
        +--------------------+-----------------+------------------+
        |                    |                 |                  |
        0       <=       readerIndex  <=   writeIndex  <=        size
        */
        class Buffer:public copyable{

            public:
                static const size_t kCheapPrepend = 8;//prependatable bytes的大小
                static const size_t kInitialSize = 1024;//缓冲区初始大小

                explicit Buffer(size_t initialSize = kInitialSize)
                    :buffer_(kCheapPrepend + initialSize),
                    readerIndex_(kCheapPrepend),
                    writeIndex_(kCheapPrepend)
                {}

                //可读取的大小
                size_t readableBytes() const{
                    return writeIndex_ - readerIndex_;
                }
                //可写缓冲区的大小
                size_t writableBytes() const{
                    return buffer_.size() - writeIndex_;
                }

                size_t prependatableBytes() const{
                    return readerIndex_;
                }
                //返回可读区的起始地址
                const char* peek() {
                    return begin()+readerIndex_;
                }
                //读取多少个
                void retrieve(size_t len){
                    if(len <= readableBytes()){
                        readerIndex_+=len;
                    }else{//如果超过了原本可读的缓冲区范围
                        retrieveAll();
                    }
                }
                //retriveAllAsString
                std::string retrieveAllAsString(){
                    retrieveAsString(readableBytes());
                }
                //retrieveAsString
                std::string retrieveAsString(size_t len){
                    std::string result(peek(),len);
                    retrieve(len);
                    return result;
                }
                //trtriveAll
                void retrieveAll(){//缓冲区重置
                    readerIndex_ = kCheapPrepend;
                    writeIndex_ = kCheapPrepend;
                }
                
                //确保有足够的可写空间
                void ensureWriteableByte(size_t len){
                    if(writableBytes()<len){
                        makeSpace(len);
                    }
                }
                //添加数据
                void append(const char* data,size_t len){
                    ensureWriteableByte(len);
                    std::copy(data,data+len,beginWrite());
                }

                const char* beginWrite() {
                    return begin()+writeIndex_;
                }

                //从fd上读取数据
                ssize_t readFd(int fd,int* saveErrno);
            private:
                //返回缓冲区的起始地址
                char* begin(){
                    return &*buffer_.begin();
                }
                
                void makeSpace(size_t len){
                    if(writableBytes()+prependatableBytes()<len+kCheapPrepend){
                        //重新指定容器的长度
                        buffer_.resize(writeIndex_+len);
                    }else{
                        //把可读数据移动到从kCheapPrepend开始的地方
                        size_t readable = readableBytes();
                        std::copy(begin()+readerIndex_,
                                  begin()+writeIndex_,
                                  begin()+kCheapPrepend);
                        readerIndex_ = kCheapPrepend;
                        writeIndex_ = readerIndex_+readable;
                    }
                }
                std::vector<char> buffer_;
                size_t readerIndex_;
                size_t writeIndex_;
        };
    }
}

#endif // !MYMUDUO_NET_BUFFER_H
