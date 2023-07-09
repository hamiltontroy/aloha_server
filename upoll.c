#include <sys/epoll.h>
#include <errno.h>
#include <stddef.h>

int upoll_create(void)
{
	return epoll_create1(0);
}

int upoll_watch(int upollfd, int fd, const char *ready_when)
{
	int when;
	struct epoll_event event;

	if(ready_when[0] == 'w')
		when = EPOLLOUT;
	else if(ready_when[0] == 'r')
		if(ready_when[1] == '+')
			when = EPOLLIN | EPOLLOUT;
		else
			when = EPOLLIN;
	else return -1;
	
	event.events = when;
	event.data.fd = fd;
	
	return epoll_ctl(upollfd, EPOLL_CTL_ADD, fd, &event);
}

int upoll_wait(int upollfd, int *ready_fd)
{
	struct epoll_event event;
	int r;
	
	WAIT:
	r = epoll_wait(upollfd, &event, 1, -1);
	if(r == -1)
	{
		if(errno == EINTR) // a signal caused premature unblocking
		{
			goto WAIT;
		}

		return -1;
	}
	
	epoll_ctl(upollfd, EPOLL_CTL_DEL, event.data.fd, NULL);
	*ready_fd = event.data.fd;
	
	return 0;
}
