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
/* 用于保存TcpConnection指针；由此来区别是否已经连接 */
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;/* 是否已经建立连接 */
}
/* 更新循环中的激活事件 */
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
    /* 已经建立连接 */
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
 * handleEventWithGuard其实就是根据不同的激活原因调用不同的回调函数，
 * 这些回调函数都在TcpConnection中，也是通过上面std::function/std::bind设置的，
 * 所以在调用前必须判断Channel所在的TcpConnection是否还存在
 * https://blog.csdn.net/djinglan/article/details/8302938
*/
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    /* 输出注册的事件 */
    LOG_TRACE << reventsToString();
    /* 发生挂起且无可读数据 */
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (logHup_)
        {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (closeCallback_) closeCallback_();
    }
    // 如果是
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
/*
POLLHUP表示套接字不再连接.在TCP中,这意味着FIN已被接收和发送.

POLLERR表示套接字出现异步错误.在TCP中,这通常意味着已经接收或发送了RST.如果文件描述符不是套接字,则POLLERR可能意味着设备不支持轮询.

对于上述两个条件,套接字文件描述符仍处于打开状态,尚未关闭(但可能已经调用了shutdown()).文件描述符上的close()将释放仍代表套接字保留的资源.理论上,应该可以立即重用套接字(例如,使用另一个connect()调用).

POLLNVAL表示套接字文件描述符未打开.关闭()它会是一个错误.
https://www.cnblogs.com/nathan-1988/archive/2012/07/01/2571786.html

注意:可读并不代表有数据
当某个socket接受缓冲区有新数据分节到达,然后select报告这个socket描述符可读,但随后,协议栈检查到这个新分节检验和错误,然后丢弃这个分节,这时候调用read()则无数据可读epoll也一样这也是为什么 
要使用非阻塞IO的原因,就算是select和epoll

作者：知乎用户
链接：https://www.zhihu.com/question/54439925/answer/298579368
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
*/
string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    // 普通优先级数据可读
    if (ev & POLLIN)
        oss << "IN ";
    // 高右下级数据可读
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    // 已经发生读挂起
    if (ev & POLLHUP)
        oss << "HUP ";
    // 发生挂起
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    // 发生错误
    if (ev & POLLERR)
        oss << "ERR ";
    // 不是一个打开的文件
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}