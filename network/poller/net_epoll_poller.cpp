

#include "net_epoll_poller.h"
#include "logging.h"
#include "net_channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop* loop)
	: Poller(loop),
		epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
		events_(kInitEventListSize)
{
	if (epollfd_ < 0)
	{
		LOG_SYSFATAL << "EPollPoller::EPollPoller";
	}
}

EPollPoller::~EPollPoller()
{
  	::close(epollfd_);
}
/* 关键poll函数 */
Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
	LOG_TRACE << "fd total count " << channels_.size();
	/* 查询满足监听事件的数目 */
	/* 返回事件集合，包括事件和fd数组 */
	int numEvents = ::epoll_wait(epollfd_,
								&*events_.begin(),
								static_cast<int>(events_.size()),
								timeoutMs);
	int savedErrno = errno;
	/* 获取当前的时间戳 */
	Timestamp now(Timestamp::now());
	if (numEvents > 0)
	{
		LOG_TRACE << numEvents << " events happened";
		/* 处理监听发生的事件 */
		fillActiveChannels(numEvents, activeChannels);
		/* 如果较多，就对events进行扩容 */
		if (implicit_cast<size_t>(numEvents) == events_.size())
		{
			events_.resize(events_.size()*2);
		}
	}
	else if (numEvents == 0)
	{
		LOG_TRACE << "nothing happened";
	}
	else
	{
		// error happens, log uncommon ones
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			LOG_SYSERR << "EPollPoller::poll()";
		}
	}
	return now;
}
/* 处理监听发生的事件 */
void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const
{
	/* 检查是否大于事件种类数目，一维一个 */
	assert(implicit_cast<size_t>(numEvents) <= events_.size());
	/* 遍历发生的事件的集合 */
	for (int i = 0; i < numEvents; ++i)
	{
		/* 获取监听到的事件 */
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		/*
		这是epoll模式epoll_event事件的数据结构，其中data不仅可以保存fd，也可以保存一个void*类型的指针。
		typedef union epoll_data {
					void    *ptr;
					int      fd;
					uint32_t u32;
					uint64_t u64;
				} epoll_data_t;
				struct epoll_event {
					uint32_t     events;    // Epoll events 
					epoll_data_t data;      //User data variable 
				};
		*/
#ifndef NDEBUG
		/* 获取文件描述符 */
		int fd = channel->fd();
		/* 查找fd是否在监听队列中，注意只有已经连接的才会在里面 */
		ChannelMap::const_iterator it = channels_.find(fd);
		/* 检查文件描述是否存在 */
		assert(it != channels_.end());
		assert(it->second == channel);
#endif
		/* 把已发生的事件传给channel,写到通道当中 */
		channel->set_revents(events_[i].events);
		/* 将其添加到已激活的事件队列中 */
		activeChannels->push_back(channel);
  }
}
/* 更新监听的事件 */
void EPollPoller::updateChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	//获取其对应的map index
	const int index = channel->index();
	LOG_TRACE << "fd = " << channel->fd()
		<< " events = " << channel->events() << " index = " << index;
	if (index == kNew || index == kDeleted)
	{
		// a new one, add with EPOLL_CTL_ADD
		int fd = channel->fd();
		if (index == kNew)
		{
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}
		else // index == kDeleted
		{
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}
		//
		channel->set_index(kAdded);
		/* 添加channel响应事件 */
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		// update existing one with EPOLL_CTL_MOD/DEL
		int fd = channel->fd();
		(void)fd;
		assert(channels_.find(fd) != channels_.end());
		assert(channels_[fd] == channel);
		assert(index == kAdded);
		if (channel->isNoneEvent())
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else
		{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}
/* 移除事件通道 */
void EPollPoller::removeChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	int fd = channel->fd();
	LOG_TRACE << "fd = " << fd;
	assert(channels_.find(fd) != channels_.end());
	assert(channels_[fd] == channel);
	assert(channel->isNoneEvent());
	int index = channel->index();
	assert(index == kAdded || index == kDeleted);
	size_t n = channels_.erase(fd);
	(void)n;
	assert(n == 1);

	if (index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}
/* 更新事件监听，包括时钟监听 */
void EPollPoller::update(int operation, Channel* channel)
{
	struct epoll_event event;
	memZero(&event, sizeof event);
	/* 将channel转换为event */
	event.events = channel->events();
	event.data.ptr = channel;
	/* 获取事件的文件描述符 */
	int fd = channel->fd();
	LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
		<< " fd = " << fd << " event = { " << channel->eventsToString() << " }";
	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
	{
		if (operation == EPOLL_CTL_DEL)
		{
			LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
		}
		else
		{
			LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
		}
	}
}

const char* EPollPoller::operationToString(int op)
{
	switch (op)
	{
		case EPOLL_CTL_ADD:
			return "ADD";
		case EPOLL_CTL_DEL:
			return "DEL";
		case EPOLL_CTL_MOD:
			return "MOD";
		default:
			assert(false && "ERROR op");
			return "Unknown Operation";
	}
}
