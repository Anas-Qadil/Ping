#include "../includes/ft_ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>  // Provides data types used in system calls
#include <sys/socket.h> // Provides socket function and data types
#include <string.h> 
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>     // Provides miscellaneous symbolic constants and types, and declares miscellaneous functions


typedef struct s_ping
{
	char *hostname; // hostname to ping
	int ttl; // time to live
	int pid; // process id
	int seq; // sequence number of the packet (starts at 0)
	int sockfd; // socket file descriptor
	struct sockaddr_in dest_addr; // destination address
	socklen_t dest_addr_len; // size of the destination address
} t_ping;

int init_ping(t_ping *ping, char *hostname) {
	struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    ping->hostname = hostname;
    ping->ttl = 64;
    ping->pid = getpid(); // get the process id
    ping->seq = 0;
    ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // create a raw socket
    if (ping->sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(hostname, NULL, &hints, &res);
    if (errcode != 0)
    {
        perror ("getaddrinfo");
        return 1;
    }

    printf ("Host: %s\n", hostname);
    while (res)
    {
        inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family)
        {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }
        inet_ntop (res->ai_family, ptr, addrstr, 100);
        printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
                addrstr, res->ai_canonname);
        res = res->ai_next;
    }

    ping->dest_addr.sin_family = AF_INET;
    ping->dest_addr.sin_port = 0;
    ping->dest_addr.sin_addr.s_addr = inet_addr(addrstr);
    if (ping->dest_addr.sin_addr.s_addr == INADDR_NONE) {
        perror("inet_addr failed");
        return 1;
    }

    ping->dest_addr_len = sizeof(ping->dest_addr);
 
    return 0;
}


void handle_sigint(int sig) {
	(void)sig;
	printf("\n--- %s ping statistics ---\n", "localhost");
	printf("%d packets transmitted, %d received, 0%% packet loss, time 0ms\n", 0, 0);
	exit(0);
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

	// signal(SIGINT, &handle_sigint);



	printf("PING %s (%s) 56(84) bytes of data.\n", av[1], av[1]);

	return 0;
}