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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cJSON.h"

char* hueDiscoverySender(char *ip, char *port);

const cJSON* hue_httpPOST(char *ip, char *URI, char *fields, cJSON *body);

char * hueAuthorize(char *ip);

#endif //HUEPI_HUEPI_H
