#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include "uncopyable.h"
#include "time_stamp.h"

#include <functional>
#include <memory>

NAMESPACE_START
namespace net
{
class EventLoop;



/**
 * 连接关键函数；这个函数，对socket描述符进行修改；
 * 只会对event和handler进行连接操作；
 * 主要负责对事件对象的分发
*/
class Channel : noncopyable
{
public:
    /* 事件回调 */
    typedef std::function<void()> EventCallback;
    /* 重设事件回调指针 */
    typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();
    /* 设置 */
    void handleEvent(Timestamp receiveTime);
    /* 设置读回调函数 */
    void setReadCallback(ReadEventCallback cb)
    { readCallback_ = std::move(cb); }
    /* 设置写回调函数 */
    void setWriteCallback(EventCallback cb)
    { writeCallback_ = std::move(cb); }
    /* 设置关闭回调 */
    void setCloseCallback(EventCallback cb)
    { closeCallback_ = std::move(cb); }
    /* 设置错误回调 */
    void setErrorCallback(EventCallback cb)
    { errorCallback_ = std::move(cb); }

    /** 
     * tie此方法是防止Channel类还在执行，上层调用导致 
     * Channel提前释放而出现的异常问题，下文会详细解释。
     * */
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }              //Channel拥有的fd
    int events() const { return events_; }      //Channel当前处理的事件类型
    void set_revents(int revt) { revents_ = revt; } // used by pollers;设置事件集合
    // 检查当前Channel是否未处理任何事件
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    //开启/关闭读事件，并更新channel信息
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    /* 开启/关闭写事件 */
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    /* 停止所有事件 */
    void disableAll() { events_ = kNoneEvent; update(); }
    /* 检查是否正在读写 */
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    /* 记录在poller中的index;方便管理 */
    int index() { return index_; }
    /* 设置poller事件 */
    void set_index(int idx) { index_ = idx; }

    // 事件信息转换，便于调试
    std::string reventsToString() const;
    std::string eventsToString() const;

    void doNotLogHup() { logHup_ = false; }
    /* 所属处理线程 */
    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    /* 将事件转换为字符串 */
    static std::string eventsToString(int fd, int ev);
    /* 更新状态 */
    void update();
    /* 设置事件处理方法类 */
    void handleEventWithGuard(Timestamp receiveTime);
    //事件
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;                               //channel所属的loop
    const int  fd_;                                 //channel负责的文件描述符
    int        events_;                             //注册的事件
    int        revents_;                            //poller设置的就绪的事件
    int        index_;                              //被poller使用的下标
    bool       logHup_;                             //是否生成某些日志

    std::weak_ptr<void> tie_;                       /* 指向connet的弱连接 */
    bool tied_;
    bool eventHandling_;                            //是否处于处理事件中
    bool addedToLoop_;
    ReadEventCallback readCallback_;                //读事件回调
    EventCallback writeCallback_;                   //写事件回调
    EventCallback closeCallback_;                   //关闭事件回调
    ReadEventCallback errorCallback_;               //错误事件回调
};
}// namespace net

NAMESPACE_END

#endif