#ifndef MYMUDUO_NET_TCPCONNECTION_H
#define MYMUDUO_NET_TCPCONNECTION_H

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

namespace mymuduo{
    
    namespace net{

        class Channel;
        class EventLoop;
        class Socket;

        /*
         * TcpConnection 用于表示已建立的TCP连接
         * 它管理连接的生命周期，处理数据的收发
         */
        class TcpConnection : noncopyable, 
                              public std::enable_shared_from_this<TcpConnection> {
            public:
                // 构造函数
                TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr);
                // 析构函数
                ~TcpConnection();

                // 获取相关信息的方法
                EventLoop* getLoop() const { return loop_; }
                const std::string& name() const { return name_; }
                const InetAddress& localAddress() const { return localAddr_; }
                const InetAddress& peerAddress() const { return peerAddr_; }
                bool connected() const { return state_ == kConnected; }
                bool disconnected() const { return state_ == kDisconnected; }

                // 发送数据
                void send(const void* message, int len);
                void send(const std::string& message);
                void send(Buffer* message);  // 这个函数会清空buffer

                // 关闭连接
                void shutdown();  // 线程安全，可以多次调用

                // 设置回调函数
                void setConnectionCallback(const ConnectionCallback& cb)
                { connectionCallback_ = cb; }
                void setMessageCallback(const MessageCallback& cb)
                { messageCallback_ = cb; }
                void setWriteCompleteCallback(const WriteCompleteCallback& cb)
                { writeCompleteCallback_ = cb; }
                void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
                { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }
                void setCloseCallback(const CloseCallback& cb)
                { closeCallback_ = cb; }

                // 连接建立和销毁
                void connectEstablished();   // 应该只被调用一次
                void connectDestroyed();     // 应该只被调用一次

                // 获取输入缓冲区
                Buffer* inputBuffer() { return &inputBuffer_; }
                
            private:
                // 连接状态
                enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
                void setState(StateE s) { state_ = s; }
                
                // 事件回调
                void handleRead(Timestamp receiveTime);
                void handleWrite();
                void handleClose();
                void handleError();

                // 发送数据的内部实现
                void sendInLoop(const void* message, size_t len);
                void sendInLoop(const std::string& message);
                
                // 关闭连接的内部实现
                void shutdownInLoop();

                // 成员变量
                EventLoop* loop_;                // 所属EventLoop
                const std::string name_;         // 连接名
                std::atomic<StateE> state_;      // 连接状态
                bool reading_;                   // 是否正在读取

                // 网络相关
                std::unique_ptr<Socket> socket_; // Socket对象
                std::unique_ptr<Channel> channel_; // Channel对象
                const InetAddress localAddr_;    // 本地地址
                const InetAddress peerAddr_;     // 对端地址

                // 回调函数
                ConnectionCallback connectionCallback_;       // 连接回调
                MessageCallback messageCallback_;             // 消息回调
                WriteCompleteCallback writeCompleteCallback_; // 写完成回调
                HighWaterMarkCallback highWaterMarkCallback_; // 高水位回调
                CloseCallback closeCallback_;                 // 关闭回调
                size_t highWaterMark_;                        // 高水位标记

                // 缓冲区
                Buffer inputBuffer_;  // 接收缓冲区
                Buffer outputBuffer_; // 发送缓冲区
        };
    }
}

#endif // !MYMUDUO_NET_TCPCONNECTION_H
