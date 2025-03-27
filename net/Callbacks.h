#ifndef MYMUDUO_NET_CALLBACKS_H
#define MYMUDUO_NET_CALLBACKS_H

#include <functional>
#include <memory>

#include "Timestamp.h"

namespace mymuduo{

    namespace net{
        class Buffer;
        class TcpConnection;

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
        using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

        using MessageCallback = std::function<void(const TcpConnectionPtr&,
                                            Buffer*,
                                            Timestamp)>;

        //在TcpConnection.cc中定义
        void defaultConnectionCallback(const TcpConnectionPtr& conn);
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer,
                                    Timestamp receiveTime);
    }
}

#endif //  MYMUDUO_NET_CALLBACKS_H
