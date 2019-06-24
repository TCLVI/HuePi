//
// Created by Tommy Lea on 6/19/19.
//

#include "huePi.h"

#define REQUESTBUFSIZE 1024

#define RESPONSEBUFSIZE 1024

//hue_httpGET creates an HTTP POST request using the URI, as well as a body, which is any number of
// http body items, separated by CRLFs. It then sends that request to the hue bridge and returns a
// JSON that is received from the hub.

const cJSON* hue_httpPOST(char *ip, char *URI, char *fields, cJSON *body)
{
    char request[REQUESTBUFSIZE];
    char *strBody = NULL;
    int spf;
    ssize_t nbytesPOST_sent;
    ssize_t nbytesPOST_recv;
    struct addrinfo hints;
    struct addrinfo *bridgeInfo;
    struct addrinfo *p;
    int selectretval;
    size_t bufpos = 0;

    fd_set readSet;
    struct timeval timeout;
    int sockisopen = 1;

    char huePort[] = "80";

    int sfd_hue = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, huePort, &hints, &bridgeInfo) == -1)
    {
        perror("ERROR - Could not get addr info");
        return NULL;
    }

    for (p = bridgeInfo; p != NULL; p = p->ai_next)
    {
        if ((sfd_hue = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("Error creating socket");
            continue;
        }

        if (connect(sfd_hue, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("Error connecting client");
            continue;
        }

        break;
    }

    freeaddrinfo(bridgeInfo);

    if (p == NULL)
    {
        perror("ERROR - Failed to create working socket");
        return NULL;
    }

    strBody = cJSON_Print(body);

    if (strBody == NULL)
    {
        perror("ERROR - Could not print JSON body into a string");
        return NULL;
    }

    if ((spf = sprintf(request, "POST %s HTTP/1.0\r\n%s\r\n%s", URI, fields, strBody)) < 1)
    {
        perror("Could not format POST request");
        return NULL;
    }

    if ((nbytesPOST_sent = send(sfd_hue, request, strlen(request), 0) == -1))
    {
        perror("ERROR - Failed to sendto");
        return NULL;
    }

    char response[RESPONSEBUFSIZE+1];

    while(sockisopen == 1)
    {
        FD_ZERO(&readSet);
        FD_SET(sfd_hue, &readSet);
        timeout.tv_sec = 5;

        selectretval = select(sfd_hue+1, &readSet, NULL, NULL, &timeout);

        //select error
        if(selectretval < 0)
        {
            perror("ERROR - SELECT FAILURE");
            return NULL;
        }

        //Socket is still open, read
        if(selectretval)
        {
            nbytesPOST_recv = recv(sfd_hue, response+bufpos, RESPONSEBUFSIZE-bufpos, 0);
            if(nbytesPOST_recv < 0)
            {
                perror("ERROR - Could not receive from socket");
                return NULL;
            }

            bufpos = nbytesPOST_recv;
        }

        //Socket is closed
        else
        {
            sockisopen = 0;
        }
    }

    //TODO: This is not the JSON RESPONSE. PARSE HTTP RESPONSES

    close(sfd_hue);
    cJSON_Delete(body);
    return NULL;
}