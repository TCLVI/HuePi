//
// Created by Thomas Lea on 6/19/19.
/*
 * httpRequests.c contains three primary functions: the first is a simple callback function used to capture the body
 * of http responses through a single file library titled "http_parser.c"
 * The other two functions are PUT and POST request functions for the philips hue API specifically. They assume that
 * the body of the requests being generated are in JSON, and require ones as such.
 */

#include "huePi.h"

#define REQUESTBUFSIZE 1024

#define RESPONSEBUFSIZE 1024

static const char *bodyBuf;

//Callback function used to capture a pointer to the body of the http response within the entire response buffer.
int my_body_callback(http_parser* parser, const char *at, size_t length)
{
    bodyBuf = at;
    return 0;
}

//HTTP POST request sender for Philips Hue API. Takes the ip address of the Philips Hue Bridge, the URI for the request,
// the header fields for the request separated by CRLFs, and the body of the request in cJSON "objects".
//Function returns a cJSON, which is the body of the response.
//Function returns NULL on error.
cJSON* hue_httpPOST(char *ip, char *URI, char *fields, cJSON *bodyReq)
{
    char request[REQUESTBUFSIZE];
    char *strBody = NULL;
    ssize_t nbytesPOST_recv;
    struct addrinfo hints;
    struct addrinfo *bridgeInfo;
    struct addrinfo *p;
    int selectretval;
    size_t bufpos = 0;
    size_t nparsed;

    http_parser_settings settings;
    http_parser_settings_init(&settings);
    settings.on_body = my_body_callback;

    http_parser *parser = malloc(sizeof(http_parser));

    fd_set readSet;
    struct timeval timeout;
    int sockisopen = 1;

    char huePort[] = "80";

    int sfd_hue = -1;

    //Set up for getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    //Populate bridgeInfo as a linked list of addrinfo structs
    if (getaddrinfo(ip, huePort, &hints, &bridgeInfo) == -1)
    {
        perror("ERROR - Could not get addr info:");
        return NULL;
    }

    //Loop through addrinfo linked list to create a TCP socket connection with Hue bridge
    for (p = bridgeInfo; p != NULL; p = p->ai_next)
    {
        if ((sfd_hue = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("ERROR - Creating socket:");
            continue;
        }

        if (connect(sfd_hue, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("ERROR - Connecting client:");
            continue;
        }

        break;
    }

    freeaddrinfo(bridgeInfo);

    if (p == NULL)
    {
        perror("ERROR - Failed to create working socket:");
        return NULL;
    }

    strBody = cJSON_Print(bodyReq);

    if (strBody == NULL)
    {
        perror("ERROR - Could not print JSON body into a string:");
        return NULL;
    }

    if (sprintf(request, "POST %s HTTP/1.0\r\n%sContent-Length:%lu\r\n\r\n%s", URI, fields, strlen(strBody), strBody) < 1)
    {
        perror("ERROR - Could not format POST request:");
        return NULL;
    }

    printf("Sending:\n%s\n\n", request);

    if (send(sfd_hue, request, strlen(request), 0) == -1)
    {
        perror("ERROR - Failed to sendto:");
        return NULL;
    }

    char response[RESPONSEBUFSIZE];
    memset(response, 0, RESPONSEBUFSIZE);

    //Loops while socket still has data to give from the HTTP response
    while(sockisopen == 1)
    {
        FD_ZERO(&readSet);
        FD_SET(sfd_hue, &readSet);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        //Select with socket on the read set only
        selectretval = select(sfd_hue+1, &readSet, NULL, NULL, &timeout);

        //select error
        if(selectretval < 0)
        {
            perror("ERROR - SELECT FAILURE:");
            return NULL;
        }

        //Socket is still open, read
        if(FD_ISSET(sfd_hue, &readSet))
        {
            //Peeking on socket
            char c;
            ssize_t peekRet = recv(sfd_hue, &c, 1, MSG_PEEK);
            {
                //Got FIN
                if(peekRet == 0)
                {
                    sockisopen = 0;
                    break;
                }

                //Error on peek
                if(peekRet < 0)
                {
                    int check = errno;
                    //Connection has been closed
                    if(errno == 54)
                    {
                        sockisopen = 0;
                        break;
                    }
                    else
                    {
                        perror("ERROR - Could not peek on socket:");
                        return NULL;
                    }
                }
            }

            //Reading from socket
            nbytesPOST_recv = recv(sfd_hue, response+bufpos, RESPONSEBUFSIZE-bufpos, 0);
            if(nbytesPOST_recv < 0)
            {
                perror("ERROR - Could not receive from socket:");
                return NULL;
            }

            bufpos += nbytesPOST_recv;
        }

        //Socket is closed
        else
        {
            sockisopen = 0;
        }
    }

    printf("POST Response: \n%s\n\n", response);

    //Set up and execute parser for the response. This will use my callback to place a pointer from the buffer to the beginning of the
    // response body in a static char* buffer called bodyBuf.
    http_parser_init(parser, HTTP_RESPONSE);
    nparsed = http_parser_execute(parser, &settings, response, bufpos);

    cJSON *bodyRes = cJSON_Parse(bodyBuf);
    if(bodyRes == NULL)
    {
        perror("ERROR - CJSON PARSE:");
        return NULL;
    }

    close(sfd_hue);
    cJSON_Delete(bodyReq);
    return bodyRes;
}