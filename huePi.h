//
// Created by Tommy Lea on 6/4/19.
//

#ifndef HUEPI_HUEPI_H
#define HUEPI_HUEPI_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

char* hueDiscoverySender(int sfd, struct addrinfo *sockInfo);

#endif //HUEPI_HUEPI_H
