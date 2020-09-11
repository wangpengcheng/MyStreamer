// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef NET_POLLER_EPOLLPOLLER_H
#define NET_POLLER_EPOLLPOLLER_H

#include "net_poller.h"
#include <vector>

struct epoll_event;

NAMESPACE_START

namespace net
{

    /*
    * 对epoll函数的封装，继承自Poller
    */
    class EPollPoller : public Poller
    {
    public:
        EPollPoller(EventLoop *loop);
        ~EPollPoller() override;

        Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
        void updateChannel(Channel *channel) override;
        void removeChannel(Channel *channel) override;

    private:
        static const int kInitEventListSize = 16; /* //默认事件数组大小，是用来装epoll_wait()返回的可读或可写事件的 */

        static const char *operationToString(int op);
        /* epoll_wait返回后将就绪的文件描述符添加到参数的激活队列中 */
        void fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const;
        /* 由updateChannel/removeChannel间接调用，执行epoll_ctl */
        void update(int operation, Channel *channel);

        typedef std::vector<struct epoll_event> EventList;

        int epollfd_;      /* epoll监听控制符 */
        EventList events_; /* epoll_event监听事件列表 */
    };

} // namespace net
NAMESPACE_END
#endif // NET_POLLER_EPOLLPOLLER_H
