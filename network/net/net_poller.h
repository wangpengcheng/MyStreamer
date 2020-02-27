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

  //不许在I/O线程中调用,I/O复用的封装
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

  //更新Channel
  virtual void updateChannel(Channel* channel) = 0;

  //移除Channel
  virtual void removeChannel(Channel* channel) = 0;

  virtual bool hasChannel(Channel* channel) const;

  static Poller* newDefaultPoller(EventLoop* loop);

  void assertInLoopThread() const
  {
    ownerLoop_->assertInLoopThread();
  }

 protected:
  typedef std::map<int, Channel*> ChannelMap;       //map关联容器的关键字为channel_所管理的fd
  ChannelMap channels_;                             //存储事件分发器的map

 private:
  EventLoop* ownerLoop_;                            //属于哪个loop
};

}  // namespace net
NAMESPACE_END

#endif  // NET_POLLER_H