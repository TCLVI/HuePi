//
// Created by Tommy Lea on 6/4/19.
/*
 * HuePi is a project/library written by Thomas Lea. It aims to give the user a simple API for controlling many of the
 * applications of a Philips Hue bridge.
 */

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
#include <errno.h>

#include "http_parser.h"
#include "cJSON.h"

char* hueDiscoverySender(char *ip, char *port);

cJSON* hue_httpPOST(char *ip, char *URI, char *fields, cJSON *body);

char * hueAuthorize(char *ip);

int my_body_callback(http_parser* parser, const char *at, size_t length);

#endif //HUEPI_HUEPI_H
