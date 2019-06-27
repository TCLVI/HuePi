//
// Created by Thomas Lea on 6/4/19.
//

#include "huePi.h"

#define MSGBUFSIZE 256

//Function takes multicast ip address and ssdp port number, and multicasts an M-SEARCH packet to discover philips hue bridges
//Function will return the IP address of the first bridge it discovers
//Returns null on error
char * hueDiscoverySender(char *ip, char *port)
{
    int sfd_mcast = -1;
    ssize_t numbytes;
    struct addrinfo hints, *bridgeInfo, *p;
    char *s = NULL;
    fd_set readSet;
    struct timeval timeout;
    int gotResponse = 0;
    int retval;

    //Set up source addr from hue hub
    struct sockaddr srcAddr;
    memset(&srcAddr, '\0', sizeof(srcAddr));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    //Populate bridgeInfo as a linked list of addrinfo structs
    if(getaddrinfo(ip, port, &hints, &bridgeInfo) == -1)
    {
        perror("ERROR - Could not get addr info:");
        return NULL;
    }

    //Loop through addrinfo linked list to create a TCP socket connection with Hue bridge
    for(p = bridgeInfo; p!= NULL; p = p->ai_next)
    {
        if((sfd_mcast = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("ERROR - Creating socket:");
            continue;
        }

        break;
    }

    freeaddrinfo(bridgeInfo);

    if(p == NULL)
    {
        perror("ERROR - Failed to create working socket:");
        return NULL;
    }

    //Set up M-Search msg with Hue bridge search-type
    char msg[] = "M-SEARCH * HTTP/1.1\n"
                 "HOST: 239.255.255.250:1900\n"
                 "MAN: ssdp:discover\n"
                 "MX: 3\n"
                 "ST: libhue:idl";

    if((sendto(sfd_mcast, msg, strlen(msg), 0, bridgeInfo->ai_addr, bridgeInfo->ai_addrlen) == -1))
    {
        perror("ERROR - Failed to sendto:");
        return NULL;
    }

    char response[MSGBUFSIZE+1];

    socklen_t len = sizeof(srcAddr);

    //Loops the multicast search with select
    //i value represents 5 seconds of timeout before multicasting the packet again
    for(int i = 0; i < 5; i++)
    {
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        FD_ZERO(&readSet);
        FD_SET(sfd_mcast, &readSet);

        retval = select(sfd_mcast+1, &readSet, NULL, NULL, &timeout);

        if(retval == -1)
        {
            perror("ERROR - Select:");
        }
        else if(retval)
        {
            gotResponse = 1;
            break;
        }

        if((sendto(sfd_mcast, msg, strlen(msg), 0, bridgeInfo->ai_addr, bridgeInfo->ai_addrlen) == -1))
        {
            perror("ERROR - Failed to sendto:");
            return NULL;
        }
    }

    if(gotResponse == 0)
    {
        printf("Discovery timeout. Closing Program...");
        return NULL;
    }

    //Receive response to capture ip address of the hub
    //NOTE: This will grab a response only from the first hue hub found on the network
    numbytes = recvfrom(sfd_mcast, response, MSGBUFSIZE, 0, &srcAddr, &len);

    if(numbytes <= 0)
    {
        perror("ERROR - Could not receive from socket:");
        return NULL;
    }

    if(numbytes > 0)
    {
        response[numbytes] = '\n';
        response[numbytes+1] = '\0';

        printf("M-SEARCH Response: \n");

        puts(response);

        s = malloc(INET_ADDRSTRLEN);

        struct sockaddr_in *srcAddr_in = (struct sockaddr_in *)&srcAddr;

        inet_ntop(AF_INET, &(srcAddr_in->sin_addr), s, INET_ADDRSTRLEN);

        return s;
    }

    return NULL;
}

//Takes ip address of hue bridge. Returns client username on success.
//Returns NULL on program error, and 0 on unsuccessful responses.
char * hueAuthorize(char *ip)
{
    printf("\nTo authorize application with Hue Bridge, press the bridge's link button\n"
           "at most 30 seconds before starting the authorization, then press enter ~$");

    getchar();

    printf("\nAuthorizing...\n\n");

    char *authoResponse = NULL;
    char fields[128] = {0};
    cJSON *devicetype = NULL;
    //cJSON *generateclientkey = NULL;

    cJSON *authoRespJSON = NULL;
    cJSON *success = NULL;
    cJSON *username = NULL;
    cJSON *error = NULL;
    cJSON *description = NULL;

    cJSON *authoJSON = cJSON_CreateObject();
    if(authoJSON == NULL)
    {
        perror("ERROR - JSON INIT:");
        return NULL;
    }

    devicetype = cJSON_CreateString("HuePi#Macbook Thomas");
    if(devicetype == NULL)
    {
        perror("ERROR - JSON INIT:");
        return NULL;
    }

    cJSON_AddItemToObject(authoJSON, "devicetype", devicetype);

    //generateclientkey = cJSON_CreateString("true");
    //if(generateclientkey == NULL)
    //{
    //    perror("ERROR - JSON INIT:");
    //    return NULL;
    //}

    //cJSON_AddItemToObject(authoJSON, "generate clientkey", generateclientkey);

    if(sprintf(fields, "Host: %s:80\r\nConnection: closed\r\nContent-Type: application/json\r\n", ip) == -1)
    {
        perror("ERROR - SPRINTF:");
        return NULL;
    }

    //s is ip address
    cJSON *authoRespJSONArray = hue_httpPOST(ip, "/api", fields, authoJSON);
    if(authoRespJSONArray == NULL)
    {
        return NULL;
    }

    authoResponse = cJSON_Print(authoRespJSONArray);

    printf("Authorization Response Body:\n\n%s\n\n", authoResponse);

    authoRespJSON = cJSON_GetArrayItem(authoRespJSONArray, 0);
    if(authoRespJSON == NULL)
    {
        perror("ERROR - JSON ARRAY ITEM GRAB:");
        return NULL;
    }

    //checks for JSON "success" if it does not exist, checks for error messages and allows user to try authorization again
    success = cJSON_GetObjectItemCaseSensitive(authoRespJSON, "success");
    if(success == NULL)
    {
        error = cJSON_GetObjectItemCaseSensitive(authoRespJSON, "error");
        {
            if(error == NULL)
            {
                perror("ERROR - Bad response:");
                return NULL;
            }
            else
            {
                description = cJSON_GetObjectItemCaseSensitive(error, "description");
                {
                    if(description == NULL)
                    {
                        perror("ERROR - RESPONSE JSON NOT FOUND:");
                        return NULL;
                    }
                    else
                    {
                        printf("SERVER - Failed authorization: %s\nPress enter to begin authorization process again."
                               " ~$", description->valuestring);

                        getchar();

                        return NULL;
                    }
                }
            }
        }
    }

    username = cJSON_GetObjectItemCaseSensitive(success, "username");
    if(username == NULL)
    {
        perror("ERROR - JSON INIT:");
        return NULL;
    }

    if(cJSON_IsString(username) && (username->valuestring != NULL))
    {
        printf("User has been authorized\n\n");
    }
    else
    {
        perror("ERROR - BAD CLIENT KEY:");
        return NULL;
    }

    cJSON_Delete(authoRespJSONArray);
    return username->valuestring;
}

