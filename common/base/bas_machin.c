/*
Jan:

This code has been tested on Solaris and Linux.

Adam Burry
aburry@igotechnologies.com
*/

#ifdef _SOLARIS_
/* needed to get SIOCGIFADDR define */
#define BSD_COMP
#define SIOCGIFHWADDR SIOCGIFADDR
#define ifr_hwaddr ifr_addr
const char* pNIC = "hme0";      // ethernet card device name
#else
/* linux */
const char* pNIC = "eth0";
#endif

#include <netinet/in.h>         // IPPROTO_IP
#include <sys/socket.h>         // socket()
#include <net/if.h>             // struct ifreq
#include <sys/ioctl.h>          // ioctl() and SIOCGIFHWADDR
#include <unistd.h>             // close()
#include <stdio.h>              // printf()
#include <time.h>               // mktime()
#include <string.h>             // strcpy()


int bMachinGetMacAddress ( unsigned char * node )
{
    struct ifreq req;
    int fdesc, err;

    // get the MAC address off the ethernet card
    fdesc = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fdesc == -1) return 0;

    strcpy(req.ifr_name, pNIC);
    err = ioctl(fdesc, SIOCGIFHWADDR, &req );
    if (err == -1) return 0;

    close(fdesc);

    memcpy(node, req.ifr_hwaddr.sa_data, 6);

    return 1;
}

int bMachinGetMacAddressStr ( char * node )
{
    char *bufpos;
    unsigned char macaddr[8];
    int i, ret;

    ret = bMachinGetMacAddress(macaddr);
    if (ret < 0)
    {
        /* failed to get mac address */
        return -1;
    };


    bufpos = node;
    for ( i = 0; i < 6; i++ )
    {
        sprintf( bufpos, "%02X ", macaddr[i] );
        bufpos += 2;
    };
    node[12] = '\0';

    return 0;
}



int bMachinCheckEndian()
{
    short x = 1;

    if ( *(char*)&x == 1)
    {	/* little-endian */
        return 0;
    }
    else
    {	/* big-endian */
        return 1;
    }
}
