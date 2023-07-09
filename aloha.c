#include "upoll.c"
#include <netinet/ip.h>
#include <unistd.h>
#include <stdio.h>

// returns -1 on error else a listening fd on success
int allocate_listening_socket(const unsigned short port_number)
{
	int listenerfd, retval;
	
	listenerfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(listenerfd < 0)
		return -1;
	
	// lets the port number be reused without an os delay
	const int enable = 1;
	retval = setsockopt(listenerfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	
	if(retval < 0)
	{
    	close(listenerfd);
		return -1;
	}
	
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	retval = bind(listenerfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

	if(retval < 0)
	{
		close(listenerfd);
		return -1;
	}
	
	retval = listen(listenerfd, 4096);
	if(retval < 0)
	{
		close(listenerfd);
		return -1;
	}
	
	return listenerfd;
}

int main(void)
{
	int listenerfd, retval, upollfd;
	const unsigned short port_number = 1024;
	
	listenerfd = allocate_listening_socket(port_number);
	if(listenerfd < 0)
	{
		printf("Error allocating listening socket on port %d\n", port_number);
		return -1;
	}
	
	upollfd = upoll_create();
	if(upollfd == -1)
	{
		printf("Error allocating upollfd\n");
		return -1;
	}
	
	retval = upoll_watch(upollfd, listenerfd, "r");
	if(retval == -1)
	{
		printf("Error upoll_watch() failed\n");
		return -1;
	}
	
	while(1)
	{
		int sockfd;
		
		retval = upoll_wait(upollfd, &sockfd);
		if(retval == -1)
		{
			printf("Error upoll_wait() failed\n");
			return -1;
		}
		
		if(sockfd == listenerfd)
		{
			int clientfd;
			
			clientfd = accept(sockfd, NULL, NULL);
			if(clientfd < 0)
			{
				printf("Error accepting client\n");
				return -1;
			}
			
			retval = upoll_watch(upollfd, clientfd, "r");
			if(retval == -1)
			{
				printf("Error upoll_watch() failed\n");
				return -1;
			}
			
			retval = upoll_watch(upollfd, sockfd, "r");
			if(retval == -1)
			{
				printf("Error upoll_watch() failed\n");
				return -1;
			}
			
			continue;		
		}
		
		char buf[1000];
		
		retval = read(sockfd, buf, 1000);
		if(retval == -1)
		{
			printf("Error read() failed\n");
			return -1;
		}
		
		buf[retval] = 0;
		
		printf("%s\n", buf);
		
		write(sockfd, "aloha", 5);
		close(sockfd);
	}
	
	return 0;
}