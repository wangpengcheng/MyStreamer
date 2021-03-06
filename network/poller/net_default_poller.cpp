
#include "net_poller.h"
#include "net_poll_poller.h"
#include "net_epoll_poller.h"
#include "logging.h"
#include <stdlib.h>

using namespace MY_NAME_SPACE::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
	if (::getenv("MY_NAME_SPACE_USE_POLL"))
	{
		return new PollPoller(loop);
	}
	else //注意这里的默认poller是epoll
	{
		LOG_INFO <<"Test: " <<loop;
		return new EPollPoller(loop);
		
	}
}
