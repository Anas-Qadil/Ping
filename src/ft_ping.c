#include "../includes/ft_ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>


typedef struct s_ping
{
	char *hostname;
	int ttl;
	int pid;
	int seq;
	int sockfd;
	struct sockaddr_in dest_addr;
	socklen_t dest_addr_len;
} t_ping;


int init_ping(t_ping *ping, char *hostname) {
	ping->hostname = hostname;
	ping->ttl = 64;
	ping->pid = getpid();
	ping->seq = 0;
	ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ping->sockfd < 0) {
		perror("socket");
		return 1;
	}
	ping->dest_addr.sin_family = AF_INET;
	ping->dest_addr.sin_port = 0;
	ping->dest_addr.sin_addr.s_addr = inet_addr(ping->hostname);
	if (ping->dest_addr.sin_addr.s_addr == INADDR_NONE) {
		perror("inet_addr");
		return 1;
	}

	ping->dest_addr_len = sizeof(ping->dest_addr);
	return 0;
}

int main(int ac, char **av) {
	if (ac < 2) {
		printf("Usage: %s <hostname>\n", av[0]);
		return 1;
	}

	t_ping ping;
	if (init_ping(&ping, av[1])) {
		return 1;
	}

	printf("PING %s (%s) 56(84) bytes of data.\n", av[1], av[1]);

	return 0;
}