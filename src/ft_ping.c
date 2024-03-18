#include "../includes/ft_ping.h"
#include <netinet/ip_icmp.h>
#define PACKET_SIZE 64
#define DATA_SIZE (PACKET_SIZE - sizeof(struct icmphdr))

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

uint16_t checksum(uint16_t *buf, int len) {
	uint32_t sum = 0;
	while (len > 1) {
		sum += *buf++;
		len -= 2;
	}
	if (len == 1) {
		sum += *(uint8_t *) buf;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (uint16_t) ~sum;
}


int dns_lookup(t_ping *ping, char *hostname) {
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(hostname, NULL, &hints, &res);
    if (errcode != 0) {
        perror("getaddrinfo");
        return 1;
    }

    printf("Host: %s\n", hostname);
    while (res) {
        inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family) {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }
        inet_ntop(res->ai_family, ptr, addrstr, 100);
        printf("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
               addrstr, res->ai_canonname ? res->ai_canonname : "N/A");
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

int init_ping(t_ping *ping, char *hostname) {
    ping->hostname = hostname;
    ping->ttl = 64; // time to live in seconds
    ping->pid = getpid(); // get the process id
    ping->seq = 0; // sequence number of the packet (starts at 0)
    ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // create a raw socket
    if (ping->sockfd < 0) {
        perror("socket");
        return 1;
    }

    return dns_lookup(ping, hostname);
}

void send_packet(t_ping *ping) {
	char packet[PACKET_SIZE];
	struct icmp *icmp = (struct icmp *) packet;

	// set the icmp header
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = ping->pid;
	icmp->icmp_seq = ping->seq++;
	memset(icmp->icmp_data, 0xa5, DATA_SIZE); // fill the packet with some data
	gettimeofday((struct timeval *) icmp->icmp_data, NULL); // set the time of the packet

	// calculate the checksum
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = checksum((uint16_t *) icmp, PACKET_SIZE);

	// send the packet
	if (sendto(ping->sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *) &ping->dest_addr, ping->dest_addr_len) <= 0) {
		perror("sendto");
	}
}

void receive_packet(t_ping *ping) {
	char packet[PACKET_SIZE];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	// receive the packet
	if (recvfrom(ping->sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *) &from, &fromlen) <= 0) {
		perror("recvfrom");
	}

	// parse the packet
	struct ip *ip = (struct ip *) packet;
	struct icmp *icmp = (struct icmp *) (packet + (ip->ip_hl << 2));

	// calculate the time taken
	struct timeval *sent_time = (struct timeval *) icmp->icmp_data;
	struct timeval now;
	gettimeofday(&now, NULL);
	long double rtt = (now.tv_sec - sent_time->tv_sec) * 1000.0 + (now.tv_usec - sent_time->tv_usec) / 1000.0;

	// print the result
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3Lf ms\n", ntohs(ip->ip_len), inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
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

	for (int i = 0; i < 3; i++) {
		// send the packet
		send_packet(&ping);
		// receive the packet
		receive_packet(&ping);
		sleep(1); // wait for 1 second
	}

	return 0;
}