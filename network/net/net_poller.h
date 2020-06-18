#ifndef NET_POLLER_H
#define NET_POLLER_H

#include <map>
#include <vector>


#include "time_stamp.h"
#include "net_event_loop.h"

NAMESPACE_START
namespace net
{

class Channel;

///
/// 纯粹的事件分发和管理工具
///
/// 它并不对channl进行管理
/**
 * 1.调用poll函数监听注册了事件的文件描述符
 * 2.当poll返回时将发生事件的事件集装入activeChannels中,并设置Channel发生事件到其revents_中
 * 3.控制channel中事件的增删改
*/
class Poller : noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  virtual ~Poller();

   /* 
   * 监听函数，对于epoll是epoll_wait，对于poll是poll 
   * 返回epoll_wait/poll返回的时间
   */
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

  //更新Channel
  virtual void updateChannel(Channel* channel) = 0;

  //移除Channel
  virtual void removeChannel(Channel* channel) = 0;

  virtual bool hasChannel(Channel* channel) const;
  /* 静态函数 */
  static Poller* newDefaultPoller(EventLoop* loop);

  void assertInLoopThread() const
  {
    ownerLoop_->assertInLoopThread();
  }

protected:
    /* 
    * Channel，保存fd和需要监听的events，以及各种回调函数（可读/可写/错误/关闭等）
    * 类似libevent的struct event
    */
    typedef std::map<int, Channel*> ChannelMap;       //map关联容器的关键字为channel_所管理的fd
    /* 保存所有事件Channel，类似libevent中base的注册队列 */
    ChannelMap channels_;                             //存储事件分发器的map

private:
    /* 
   * EventLoop，事件驱动主循环，用于调用poll函数
   * 类似libevent的struct event_base
   */
    EventLoop* ownerLoop_;                            //属于哪个loop
};

}  // namespace net
NAMESPACE_END

#endif  // NET_POLLER_H