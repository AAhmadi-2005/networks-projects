#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

int main()
{
    int sockfd;

    unsigned char buffer[2048]; // in yani har kodom 1 byte shodan 
    // (char mamomili bit adada alamat dar mikhone )
    
    struct sockaddr_ll phyaddr;
    socklen_t addrlen;

    ssize_t packet_len;

    sockfd = socket(PF_PACKET , SOCK_RAW , htons(ETH_P_ALL));
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

   while (1)
    {
        addrlen = sizeof(phyaddr);

        packet_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&phyaddr, &addrlen);

        if (packet_len < 0)
        {
            perror("recvfrom");
            continue;
        }

        switch (phyaddr.sll_pkttype)
        {
            case PACKET_HOST:
                printf("Incoming:\n");
                break;

            case PACKET_OUTGOING:
                printf("Outgoing:\n");
                break;

            case PACKET_BROADCAST:
                printf("Broadcast:\n");
                break;

            case PACKET_MULTICAST:
                printf("Multicast:\n");
                break;
        }
        for (int i = 0; i < packet_len; i++)
        {
            printf("%02X", buffer[i]); // in baray ine ke tak raghami ha ra 1ki chap nakone 
            // yani bejay 0 00 chap mikone
        }

        printf("\n");
    }

    close(sockfd);

    return 0;
}