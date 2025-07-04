#include "TcpConnection.h"
#include "Callbacks.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>
#include <functional>

namespace mymuduo{

    namespace net{

        void defaultConnectionCallback(const TcpConnectionPtr& conn){
            LOG_INFO("TcpConnection::defaultConnectionCallback - %s -> %s is %s",
                    conn->localAddress().toIpPort().c_str(),
                    conn->peerAddress().toIpPort().c_str(),
                    (conn->connected() ? "UP" : "DOWN"));
        }
        
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer,
                                    Timestamp receiveTime){
            buffer->retrieveAll();
        }

        TcpConnection::TcpConnection(EventLoop* loop,
                                    const std::string& nameArg,
                                    int sockfd,
                                    const InetAddress& localAddr,
                                    const InetAddress& peerAddr)
            : loop_(loop)
            , name_(nameArg)
            , state_(kConnecting)
            , reading_(true)
            , socket_(new Socket(sockfd))
            , channel_(new Channel(loop, sockfd))
            , localAddr_(localAddr)
            , peerAddr_(peerAddr)
            , highWaterMark_(64*1024*1024) // 64MB
        {
            // 设置channel的回调函数
            channel_->setReadCallback(
                std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
            channel_->setWriteCallback(
                std::bind(&TcpConnection::handleWrite, this));
            channel_->setCloseCallback(
                std::bind(&TcpConnection::handleClose, this));
            channel_->setErrorCallback(
                std::bind(&TcpConnection::handleError, this));

            LOG_INFO("TcpConnection::ctor[%s] at fd=%d", name_.c_str(), sockfd);
            socket_->setKeepAlive(true);
        }

        TcpConnection::~TcpConnection()
        {
            LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d", 
                    name_.c_str(), channel_->fd(), (int)state_);
        }

        void TcpConnection::send(const void* data, int len)
        {
            if (state_ == kConnected)
            {
                if (loop_->isInLoopThread())
                {
                    sendInLoop(data, len);
                }
                else
                {
                    void (TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
                    loop_->runInLoop(
                        std::bind(fp, 
                                this,     // FIXME
                                data,
                                len));
                }
            }
        }

        void TcpConnection::send(const std::string& message)
        {
            if (state_ == kConnected)
            {
                if (loop_->isInLoopThread())
                {
                    sendInLoop(message);
                }
                else
                {
                    void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
                    loop_->runInLoop(
                        std::bind(fp,
                                this,
                                message));
                }
            }
        }

        void TcpConnection::send(Buffer* buf)
        {
            if (state_ == kConnected)
            {
                if (loop_->isInLoopThread())
                {
                    sendInLoop(buf->peek(), buf->readableBytes());
                    buf->retrieveAll();
                }
                else
                {
                    void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
                    loop_->runInLoop(
                        std::bind(fp,
                                this,
                                buf->retrieveAllAsString()));
                }
            }
        }

        void TcpConnection::sendInLoop(const void* data, size_t len)
        {
            loop_->assertInLoopThread();
            ssize_t nwrote = 0;
            size_t remaining = len;
            bool faultError = false;

            // 如果连接已经关闭，直接返回
            if (state_ == kDisconnected)
            {
                LOG_WARN("disconnected, give up writing");
                return;
            }

            // 如果channel没有关注可写事件，且输出缓冲区没有数据，尝试直接发送
            if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
            {
                nwrote = sockets::write(channel_->fd(), data, len);
                if (nwrote >= 0)
                {
                    remaining = len - nwrote;
                    if (remaining == 0 && writeCompleteCallback_)
                    {
                        // 数据全部发送完成，调用写完成回调
                        loop_->queueInLoop(
                            std::bind(writeCompleteCallback_, shared_from_this()));
                    }
                }
                else // nwrote < 0
                {
                    nwrote = 0;
                    if (errno != EWOULDBLOCK)
                    {
                        LOG_ERROR("TcpConnection::sendInLoop");
                        if (errno == EPIPE || errno == ECONNRESET)
                        {
                            faultError = true;
                        }
                    }
                }
            }

            // 如果没有全部发送完，将剩余数据添加到输出缓冲区，并关注可写事件
            if (!faultError && remaining > 0)
            {
                size_t oldLen = outputBuffer_.readableBytes();
                if (oldLen + remaining >= highWaterMark_
                    && oldLen < highWaterMark_
                    && highWaterMarkCallback_)
                {
                    loop_->queueInLoop(
                        std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
                }
                outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
                if (!channel_->isWriting())
                {
                    channel_->enableWriting();
                }
            }
        }

        void TcpConnection::sendInLoop(const std::string& message)
        {
            sendInLoop(message.data(), message.size());
        }

        void TcpConnection::shutdown()
        {
            if (state_ == kConnected)
            {
                setState(kDisconnecting);
                loop_->runInLoop(
                    std::bind(&TcpConnection::shutdownInLoop, this));
            }
        }

        void TcpConnection::shutdownInLoop()
        {
            loop_->assertInLoopThread();
            if (!channel_->isWriting())
            {
                socket_->shutdownWrite();
            }
        }

        void TcpConnection::connectEstablished()
        {
            loop_->assertInLoopThread();
            assert(state_ == kConnecting);
            setState(kConnected);
            
            // 将TcpConnection的生命周期与Channel绑定
            channel_->tie(shared_from_this());
            channel_->enableReading();

            // 调用用户设置的连接回调
            connectionCallback_(shared_from_this());
        }

        void TcpConnection::connectDestroyed()
        {
            loop_->assertInLoopThread();
            if (state_ == kConnected)
            {
                setState(kDisconnected);
                channel_->disablaAll();

                connectionCallback_(shared_from_this());
            }
            channel_->remove();
        }

        void TcpConnection::handleRead(Timestamp receiveTime)
        {
            loop_->assertInLoopThread();
            int savedErrno = 0;
            ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
            if (n > 0)
            {
                // 有数据可读，调用用户设置的消息回调
                messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
            }
            else if (n == 0)
            {
                // 对端关闭连接
                handleClose();
            }
            else
            {
                // 出错
                errno = savedErrno;
                LOG_ERROR("TcpConnection::handleRead");
                handleError();
            }
        }

        void TcpConnection::handleWrite()
        {
            loop_->assertInLoopThread();
            if (channel_->isWriting())
            {
                ssize_t n = sockets::write(channel_->fd(),
                                        outputBuffer_.peek(),
                                        outputBuffer_.readableBytes());
                if (n > 0)
                {
                    outputBuffer_.retrieve(n);
                    if (outputBuffer_.readableBytes() == 0)
                    {
                        // 数据全部发送完毕，停止关注可写事件
                        channel_->disableWriting();
                        if (writeCompleteCallback_)
                        {
                            // 调用写完成回调
                            loop_->queueInLoop(
                                std::bind(writeCompleteCallback_, shared_from_this()));
                        }
                        if (state_ == kDisconnecting)
                        {
                            // 如果状态是正在断开连接，且数据已全部发送完毕，则关闭连接
                            shutdownInLoop();
                        }
                    }
                }
                else
                {
                    LOG_ERROR("TcpConnection::handleWrite");
                }
            }
            else
            {
                LOG_TRACE("Connection fd = %d is down, no more writing", channel_->fd());
            }
        }

        void TcpConnection::handleClose()
        {
            loop_->assertInLoopThread();
            LOG_TRACE("fd = %d state = %d", channel_->fd(), (int)state_);
            assert(state_ == kConnected || state_ == kDisconnecting);
            
            // 不再关注任何事件
            setState(kDisconnected);
            channel_->disablaAll();

            TcpConnectionPtr guardThis(shared_from_this());
            connectionCallback_(guardThis);  // 通知用户连接已关闭
            closeCallback_(guardThis);       // 通知所有者（TcpServer）移除此连接
        }

        void TcpConnection::handleError()
        {
            int err = sockets::getSocketError(channel_->fd());
            LOG_ERROR("TcpConnection::handleError [%s] - SO_ERROR = %d %s",
                    name_.c_str(), err, strerror(err));
        }
    }
}
