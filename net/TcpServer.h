#ifndef MYMUDUO_NET_TCPSERVER_H
#define MYMUDUO_NET_TCPSERVER_H

#include "noncopyable.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <atomic>
namespace mymuduo{

    namespace net{

        class TcpServer:noncopyable{
            public:
                //是否使用端口复用
                enum Option
                {
                    kNoReUsePort,
                    kReUsePort,
                };

                using ThreadInitCallback = std::function<void(EventLoop*)>;
                
                //这里的listenAddr会被mainReactor用来创建监听套接字
                TcpServer(EventLoop* loop,
                        const InetAddress& listenAddr,
                        const std::string& nameArg,
                        Option option = kNoReUsePort
                        );
                ~TcpServer();

                const std::string& ipPort() const {return ipPort_;}
                const std::string& name() const {return name_;}
                EventLoop* getLoop() const { return loop_;}

                /*void setThreadNum(int numThreads);
                作用：设置服务器的事件循环线程池（EventLoopThreadPool）的大小
                参数：
                    0：所有I/O操作都在主线程（即TcpServer创建时所在的EventLoop）中处理，不启用线程池。
                    大于0的值（例如4），则会创建指定数量的线程，每个线程运行一个独立的EventLoop（这里是subloop，不包括mainloop），用于分担连接的I/O处理。
                */
                void setThreadNum(int numThreads);

                void setThreadInitCallback(const ThreadInitCallback& cb)
                {threadInitCallback_ = cb;}

                std::shared_ptr<EventLoopThreadPool> threadPool()
                { return threadPool_;}
                
                //启动服务器
                void start();

                void setConnectionCallback(const ConnectionCallback& cb)
                { connectionCallback_ = cb;}
                void setMessageCallback(const MessageCallback& cb)
                { messageCallback_ = cb;}
                void setWriteCompleteCallback(const WriteCompleteCallback& cb)
                { writeCompleteCallback_ = cb;}
            private:
            /*void newConnection(int sockfd,const InetAddress& peerAddr);
                作用：处理新建立的Tcp连接
                参数：
                    sockfd：服务器与客户端之间连接的sockfd
                    peerAddr：对端（客户端）的InetAddress，包含IP和端口号
                功能：
                    通常由TcpServer在接受连接（通过accept系统调用）后调用
                    负责创建TcpConnection对象，将sockfd和peerAddr绑定到该对象，并调用相关回调（如消息接受 连接关闭等）
                    之后，新的TcpConnection会被加入到事件循环（EventLoop）中，用于后续的异步IO
                使用场景：当服务器检测到有新的客户端连接时触发，用于初始化连接并开始监听该连接上的事件。
                */
                void newConnection(int sockfd,const InetAddress& peerAddr);

                /*void removeConnection(const TcpConnectionPtr& conn);
                作用：从服务器中移除一个现有的TCP连接。
                参数：
                    conn : 一个TcpConnectionPtr（智能指针类型），指向需要移除的TcpConnection对象。
                功能：
                    这个函数用于安全地移除一个TCP连接，但它并不直接在调用线程中销毁连接对象。
                    通常会将移除操作委托给连接所属的EventLoop线程（通过跨线程调用），以避免多线程竞争问题。
                    调用后，连接的状态会被标记为“关闭中”，并最终由事件循环线程完成清理（如关闭sockfd、释放资源等）。
                使用场景：
                    当客户端主动断开连接或服务器决定关闭某个连接时调用。
                */
                void removeConnection(const TcpConnectionPtr& conn);

                /*void removeConnectionInLoop(const TcpConnectionPtr& conn);
                作用：在事件循环线程中直接移除一个TCP连接。
                参数：
                    指向需要移除的TcpConnection对象。
                功能：
                    与removeConnection不同，这个函数是在连接所属的EventLoop线程中直接执行的，因此不会有跨线程调用的开销。
                    它会立即执行连接的清理工作，包括关闭文件描述符、移除事件监听、销毁TcpConnection对象等。
                    这是removeConnection的底层实现，通常由EventLoop内部调用。
                使用场景：
                    在事件循环内部处理连接关闭时使用，用户一般不会直接调用这个函数，而是通过removeConnection间接触发。
                */
                void removeConnectionInLoop(const TcpConnectionPtr& conn);

                using ConnectionMap = std::unordered_map<std::string,TcpConnectionPtr>;

                EventLoop* loop_;
                const std::string ipPort_;
                const std::string name_;
                std::unique_ptr<Acceptor> acceptor_;
                std::shared_ptr<EventLoopThreadPool> threadPool_;
                ConnectionCallback connectionCallback_;
                MessageCallback messageCallback_;
                WriteCompleteCallback writeCompleteCallback_;
                ThreadInitCallback threadInitCallback_;
                
                //状态标志，0表示服务器未启动，1表示已经启动（可能有其他值，取决于具体情况）
                //它的主要用途是防止TcpServer被重复启动或在未启动时被错误操作。
                //因为TcpServer可能在多线程环境中使用（例如，一个线程创建服务器，另
                //一个线程启动或停止它），AtomicInt32确保状态检查和修改是原子操作，
                //避免了线程竞争导致的状态不一致。
                std::atomic_int started_;

                ConnectionMap connections_;

                int nextConnId_;
        };
    }
}

#endif // !MYMUDUO_NET_TCPSERVER_H

