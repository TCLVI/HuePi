//
// Created by Tommy Lea on 6/4/19.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGBUFSIZE 256

//Takes a socket file descriptor that is open to the multicast address on the ssdp port, and a pointer to the corresponding
// addrinfo struct. Returns the IP address of the hue bridge after multicasting an M-SEARCH request out and reading back responses
// from the bridge.
char* hueDiscoverySender(int sfd, struct addrinfo *sockInfo)
{
    char *s = NULL;

    //Set up M-Search msg

    char msg[] = "M-SEARCH * HTTP/1.1\n"
                 "HOST: 239.255.255.250:1900\n"
                 "MAN: ssdp:discover\n"
                 "MX: 3\n"
                 "ST: libhue:idl";
    int numbytes;

    if((numbytes = sendto(sfd, msg, strlen(msg), 0, sockInfo->ai_addr, sockInfo->ai_addrlen) == -1))
    {
        perror("ERROR - Failed to sendto");
        exit(1);
    }

    //Set up source addr from hue hub
    struct sockaddr srcAddr;
    memset(&srcAddr, '\0', sizeof(srcAddr));

    char response[MSGBUFSIZE+1];

    socklen_t len = sizeof(srcAddr);

    numbytes = recvfrom(sfd, response, MSGBUFSIZE, 0, &srcAddr, &len);

    if(numbytes < 0)
    {
        perror("ERROR - Could not receive from socket");
        exit(1);
    }

    if(numbytes > 0)
    {
        response[numbytes] = '\n';
        response[numbytes+1] = '\0';
        puts(response);

        s = malloc(INET_ADDRSTRLEN);

        struct sockaddr_in *srcAddr_in = (struct sockaddr_in *)&srcAddr;

        inet_ntop(AF_INET, &(srcAddr_in->sin_addr), s, INET_ADDRSTRLEN);

        return s;
    }

    return NULL;
}

