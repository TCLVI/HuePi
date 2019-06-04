#include <stdio.h>
#include "huePi.h"

int main(int argc, char **argv)
{
    printf("Multicasting in SSDP port on local network...\n");
    multicastListener();

    return 0;
}