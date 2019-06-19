//
// Created by Tommy Lea on 6/4/19.
/*
 * This is the main driver for running and testing the philips hue bridge controller application
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huePi.h"

int main(int argc, char **argv)
{
    //Initializes multicast group IP address
    char mCastGroup[] = "239.255.255.250";

    //Initializes SSDP port for multicast group. Hue bridges use SSDP for connections.
    char ssdpPort[] = "1900";

    int sfd;
    int numbytes;
    struct addrinfo hints, *bridgeInfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if(getaddrinfo(mCastGroup, ssdpPort, &hints, &bridgeInfo) == -1)
    {
        perror("ERROR - Could not get addr info");
        exit(1);
    }

    for(p = bridgeInfo; p!= NULL; p = p->ai_next)
    {
        if((sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("Error creating socket");
            continue;
        }

        break;
    }

    if(p == NULL)
    {
        perror("ERROR - Failed to create working socket");
        exit(1);
    }

    printf("Multicasting in SSDP port on local network...\n");

    char *s = NULL;

    s = hueDiscoverySender(sfd, p);

    printf("IP address is %s\n", s);

    free(s);

    return 0;
}