//
// Created by Thomas Lea on 6/4/19.
/*
 * This is the main driver for running and testing the philips hue bridge controller application
 */

#include "huePi.h"

int main(int argc, char **argv)
{

    // ************************* SET UP ********************************
    //Initializes multicast group IP address
    char mCastGroup[] = "239.255.255.250";

    //Initializes SSDP port for multicast group. Hue bridges use SSDP for connections.
    char ssdpPort[] = "1900";

    //********************** GETTING IP ADDRESS ***********************

    printf("Multicasting in SSDP port on local network...\n\n");

    char *hueIP = NULL;

    //Captures bridge IP address
    hueIP = hueDiscoverySender(mCastGroup, ssdpPort);
    if(hueIP == NULL)
    {
        return -1;
    }

    printf("IP address is %s\n", hueIP);

    //********************* AUTHORIZATION ***********************

    char *clientkey;

    //Authorizes application with the bridge, gets username for future API calls
    clientkey = hueAuthorize(hueIP);
    //Checks if program had error, lets the user try again.
    while(clientkey == NULL)
    {
        clientkey = hueAuthorize(hueIP);
    }

    printf("Client key is %s\n\n", clientkey);

    return 0;
}