//
// Created by Tommy Lea on 6/4/19.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGBUFSIZE 256

void multicastListener()
{
    //Initializes multicast group IP address
    char mCastGroup[15] = "239.255.255.250";

    //Initializes SSDP port for multicast group. Hue bridges use SSDP for connections.
    int ssdpPort = 1900;

    //Initializes ipv4 UDP socket for multicasting
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        perror("ERROR - Socket could not be created");
        return;
    }

    //Allow multiple sockets to use the same port number, this is done since the program is accepting
    //UDP connections from any address on the local network, this is done with SO_REUSEADDR
    //SOL_SOCKET sets the option change to occur on the socket level of the protocol levels
    u_int y = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &y, sizeof(y)) < 0)
    {
        perror("ERROR - Socket cannot reuse local addr");
        return;
    }

    //Set up destination address. Uses INADDR_ANY to accept all incoming SSDP UDP connections on the local network.
    struct sockaddr_in addr;

    //empty the structs memory
    memset(&addr, 0, sizeof(addr));

    //IPv4
    addr.sin_family = AF_INET;

    //Destination address is any on the local network that can accept packets on the SSDP port
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Sets up SSDP port for socket
    addr.sin_port = htons(ssdpPort);

    //bind to receive address
    if(bind(sfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        perror("ERROR - Failed to bind socket");
        return;
    }

    //Make socket request to join multicast group, uses ip_mreq struct to set up multicast group request
    struct ip_mreq mreq;

    //converts multicast group address from numbers and dots to binary address for the struct
    mreq.imr_multiaddr.s_addr = inet_addr(mCastGroup);

    //Requires local interface address
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0)
    {
        perror("ERROR - Failed kernel request to join multicast group");
        return;
    }

    //Read and print from socket loop
    while(1)
    {
        char msgbuf[MSGBUFSIZE];
        socklen_t addrlen = sizeof(addr);

        //Reads from socket nbytes number of bytes
        ssize_t nbytes = recvfrom(sfd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr*) &addr, &addrlen);

        if(nbytes < 0)
        {
            perror("ERROR - Could not read from socket");
            return;
        }

        //Null terminate the buffer for puts()
        msgbuf[nbytes] = '\0';
        puts(msgbuf);
    }

    return;
}

