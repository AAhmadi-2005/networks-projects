#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

int main()
{
    int sockfd;

    unsigned char buffer[2048];

    struct sockaddr_ll phyaddr;
    socklen_t addrlen;

    ssize_t packet_len;

    struct ethhdr *eth;
    struct iphdr *ip;

    sockfd = socket( PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    while (1)
    {
        addrlen = sizeof(phyaddr);

        packet_len = recvfrom(sockfd,buffer, sizeof(buffer),0, (struct sockaddr *)&phyaddr, &addrlen);

        if (packet_len < 0)
        {
            perror("recvfrom");
            continue;
        }

        eth = (struct ethhdr *)buffer;

        printf("Upper Protocol: ");

        switch (ntohs(eth->h_proto))
        {
            case ETH_P_IP:
                printf("IP ");
                break;

            case ETH_P_ARP:
                printf("ARP ");
                break;
        }

        if (ntohs(eth->h_proto) == ETH_P_IP)
        {
            ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
            // inja mikhaym header linka rad konim faghat header ip ra bekhonim
            printf("(header len: %d,", ip->ihl * 4);

            printf("total len: %d,", ntohs(ip->tot_len));

            printf("proto: %d),", ip->protocol);
        }


        switch (phyaddr.sll_pkttype)
        {
            case PACKET_HOST:
                printf(" Incoming:");
                break;

            case PACKET_OUTGOING:
                printf(" Outgoing:");
                break;

            case PACKET_BROADCAST:
                printf(" Broadcast:");
                break;

            case PACKET_MULTICAST:
                printf(" Multicast:");
                break;
        }

        printf("\n");

        for (int i = 0; i < packet_len; i++)
        {
            printf("%02X", buffer[i]);// in serfan baray ghashangtar khondaneshe
        }

        printf("\n");
    }

    close(sockfd);

    return 0;
}