#ifndef NET_CONNECTOR_H
#define NET_CONNECTOR_H
#include "uncopyable.h"
#include "net_inet_address.h"
#include "logging.h"

#include <functional>
#include <memory>
NAMESPACE_START

namespace net
{
    class Channel;
    class EventLoop;
    /**
     * 
     * 对应客户端的连接函数；
     * 主要记录连接状态和连接回调函数
     * connet本质上只是记录状态和回调函数
     * 当前链接的状态和回调函数
     */
    class Connector : noncopyable,
                      public std::enable_shared_from_this<Connector>
    {
    public:
        /* 连接回调处理函数，主要是处理socket fd */
        typedef std::function<void(int sockfd)> NewConnectionCallback;
        /**
         * @brief Construct a new Connector object
         * @param  loop             绑定的事件循环
         * @param  serverAddr       服务器地址
         */
        Connector(EventLoop *loop, const InetAddress &serverAddr);
        ~Connector();

        void setNewConnectionCallback(const NewConnectionCallback &cb)
        {
            newConnectionCallback_ = cb;
        }

        void start();   // can be called in any thread
        void restart(); // must be called in loop thread
        void stop();    // can be called in any thread

        const InetAddress &serverAddress() const { return serverAddr_; }

    private:
        enum States
        {
            kDisconnected, /* 没有连接 */
            kConnecting,   /* 正在连接 */
            kConnected     /* 已经连接 */
        };
        /* 设置最大重试延迟时间 */
        static const int kMaxRetryDelayMs = 30 * 1000;
        /* 最大初始化时间,连接最大等待时间 */
        static const int kInitRetryDelayMs = 500;
        /**
         * @brief Set the State object
         * @param  s                当前的连接状态  
         * 
         */
        void setState(States s) { state_ = s; }
        /* 开启/停止事件循环 */
        void startInLoop();
        void stopInLoop();
        /* 连接 */
        void connect();
        void connecting(int sockfd);
        /* 写入句柄 */
        void handleWrite();
        void handleError();
        /* 重新连接 */
        void retry(int sockfd);
        /* 移除或者重设事件 */
        int removeAndResetChannel();
        void resetChannel();
        /* 绑定的loop */
        EventLoop *loop_;
        /* 服务器地址 */
        InetAddress serverAddr_;
        /* 是否连接 */
        bool connect_; // atomic
        States state_; // FIXME: use atomic variable
        /*  对应的事件 */
        std::unique_ptr<Channel> channel_;
        /* 连接回调 */
        NewConnectionCallback newConnectionCallback_;

        /* 延迟时间 */
        int retryDelayMs_;
    };

} // namespace net

NAMESPACE_END

#endif