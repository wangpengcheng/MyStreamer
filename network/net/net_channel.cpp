#include "logging.h"
#include "net_channel.h"
#include "net_event_loop.h"

#include <sstream>
#include <poll.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;
/* 初始化事件集合 */

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd__)
  : loop_(loop),
    fd_(fd__),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}
/* 
 * 由TcpConnection中connectEstablished函数调用
 * 作用是当建立起一个Tcp连接后，让用于监测fd的Channel保存这个Tcp连接的弱引用
 * tie_是weak_ptr的原因
 *      weak_ptr是弱引用，不增加引用基数，当需要调用内部指针时
 *      可通过lock函数提升成一个shared_ptr，如果内部的指针已经销毁
 *      那么提升的shared_ptr是null
 *      可以通过是否是null判断Tcp是否还处于连接，因为如果断开，那么这个TcpConnection就被销毁了
 */
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}
//移除事件分发器
void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}
//核心：事件处理函数
/*
 * 根据fd激活事件的不同，调用不同的fd的回调函数
 */
void Channel::handleEvent(Timestamp receiveTime)
{
    /* 
    * RAII，对象管理资源
    * weak_ptr使用lock提升成shared_ptr，此时引用计数加一
    * 函数返回，栈空间对象销毁，提升的shared_ptr guard销毁，引用计数减一
    */
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);//调用handleEventWithGuard
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}
/**
 * handleEventWithGuard其实就是根据不同的激活原因滴啊用不同的回调函数，
 * 这些回调函数都在TcpConnection中，也是通过上面std::function/std::bind设置的，
 * 所以在调用前必须判断Channel所在的TcpConnection是否还存在
*/
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    /* 输出注册的事件 */
    LOG_TRACE << reventsToString();
    /* 关闭事件 */
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (logHup_)
        {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & POLLNVAL)
    {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }
    //错误件处理
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_) readCallback_(receiveTime);
    }
    //写事件处理
    if (revents_ & POLLOUT)
    {
        if (writeCallback_) writeCallback_();
    }
    /* 事件已经处理完毕 */
    eventHandling_ = false;
}

string Channel::reventsToString() const
{
    return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
    return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}