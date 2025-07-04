#include "TcpServer.h"
#include "Logging.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "SocketsOps.h"
#include "TcpConnection.h"

#include <functional>
#include <string>
#include <cassert>

namespace mymuduo{

    namespace net{

        //这里的listenAddr会被mainReactor用来创建监听套接字
        TcpServer::TcpServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& nameArg,
        Option option
        )
        :loop_(CheckNotNull<EventLoop>(loop)),
        ipPort_(listenAddr.toIpPort()),
        name_(nameArg),
        acceptor_(new Acceptor(loop,listenAddr,option == kNoReUsePort)),
        threadPool_(new EventLoopThreadPool(loop,name_)),
        connectionCallback_(defaultConnectionCallback),
        messageCallback_(defaultMessageCallback),
        started_(0),
        nextConnId_(1)
        {
            // 设置Acceptor的新连接回调函数
            acceptor_->setNewConnectionCallback(
                std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
        }

        TcpServer::~TcpServer()
        {
            loop_->assertInLoopThread();
            LOG_INFO("TcpServer::~TcpServer [%s] destructing", name_.c_str());

            for (auto& item : connections_)
            {
                TcpConnectionPtr conn(item.second);
                item.second.reset();
                conn->getLoop()->runInLoop(
                    std::bind(&TcpConnection::connectDestroyed, conn));
            }
        }

        void TcpServer::setThreadNum(int numThreads)
        {
            assert(numThreads >= 0);
            threadPool_->setThreadNum(numThreads);
        }

        void TcpServer::start()
        {
            if (started_.fetch_add(1) == 0)
            {
                threadPool_->start(threadInitCallback_);
                assert(!acceptor_->listening());
                loop_->runInLoop(
                    std::bind(&Acceptor::listen, acceptor_.get()));
            }
        }

        void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr){
            loop_->assertInLoopThread();
            EventLoop* ioLoop = threadPool_->getNextLoop();
            char buf[64];
            snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
            ++nextConnId_;
            std::string connName = name_ + buf;

            LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s",
                    name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

            // 创建一个localAddr
            InetAddress localAddr(sockets::getLocalAddr(sockfd));

            // 创建一个新的TcpConnection对象
            TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                                connName,
                                                sockfd,
                                                localAddr,
                                                peerAddr));

            connections_[connName] = conn;
            conn->setConnectionCallback(connectionCallback_);
            conn->setMessageCallback(messageCallback_);
            conn->setWriteCompleteCallback(writeCompleteCallback_);
            
            // 设置关闭回调，用于从TcpServer中移除TcpConnection
            conn->setCloseCallback(
                std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
            
            // 在IO线程中建立连接
            ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
        }

        void TcpServer::removeConnection(const TcpConnectionPtr& conn)
        {
            // 在主Loop中移除连接
            loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
        }

        void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
        {
            loop_->assertInLoopThread();
            LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s",
                    name_.c_str(), conn->name().c_str());
            
            size_t n = connections_.erase(conn->name());
            assert(n == 1); (void)n;
            
            EventLoop* ioLoop = conn->getLoop();
            ioLoop->queueInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
        }
    }
}
