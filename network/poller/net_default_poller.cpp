
#include "net_poller.h"
#include "net_poll_poller.h"
#include "net_epoll_poller.h"
#include <stdlib.h>

using namespace MY_NAME_SPACE::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
	if (::getenv("MY_NAME_SPACE_USE_POLL"))
	{
		return new PollPoller(loop);
	}
	else
	{
		return new EPollPoller(loop);
	}
}
