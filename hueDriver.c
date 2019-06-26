//
// Created by Tommy Lea on 6/4/19.
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

    hueIP = hueDiscoverySender(mCastGroup, ssdpPort);
    if(hueIP == NULL)
    {
        return -1;
    }

    printf("IP address is %s\n\n", hueIP);

    //********************* AUTHORIZATION ***********************

    char *clientkey;

    printf("To authorize application with Hue Bridge, press the bridge's link button\n"
           "at most 30 seconds before starting the authorization, then press enter ~$");

    getchar();

    printf("\nAuthorizing...\n\n");

    clientkey = hueAuthorize(hueIP);
    if(clientkey == NULL)
    {
        return -1;
    }

    printf("Client key is %s\n\n", clientkey);

    return 0;
}