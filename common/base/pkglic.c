#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
lic format
=========================
char  type;
chat  ver;
unsigned char  key[12];

*/

typedef struct xxLicType
{
    char  type; /* 'A' = logq*/
    char  ver;  /* '1' = demo 30일, '0' = 정식 */
    char  key[12];
} xxLicType, *xxLicTypePtr;

static char *seed = "adnkeyadnkeyadnkeyadnkey";

int CryptEncodeKey(char* sec, char* sour, int slen, char* seed, int dlen);
int CryptDecodeKey(char* sour, char* sec, int slen, char* seed, int dlen);
int bMachinGetMacAddressStr(char * node);
int PkgLicVelifyMacAddr(char *in_mac);
int PkgLicVelifyDemoData(char *date);
long bGet_CurSecond();
long bGet_SecFromTimeFmt(char *stime);

int PkgLicGenKey(char *out_buff, char type, char ver, char *data)
{
//    unsigned char buff[128];
    char buff[128];
    int len;

    sprintf(buff, "%c%c%s", type, ver, data);

    len = CryptEncodeKey(out_buff, buff, strlen(buff), seed, strlen(seed));
    out_buff[len] = '\0';

    return 0;
}

int PkgLicCheckKey(char *in_buff, char *mesg)
{
    char buff[128];
    int len, ret;
    xxLicTypePtr key = (xxLicTypePtr)&buff;

    memset(buff, '\0', 128);;

    len = CryptDecodeKey(buff, in_buff, strlen(in_buff), seed, strlen(seed));
    in_buff[len] = '\0';

    /*
    *type = key->type;
    *ver  = key->ver;
    strcpy(data, key->key);
    */

    if ( toupper(key->type) != 'A')
    {
        bMachinGetMacAddressStr(buff);
        sprintf(mesg, "invalid key.[%s]", buff);
        return -1;
    };

    switch (key->ver)
    {
    case '0': /* check mac address */
        ret = PkgLicVelifyMacAddr(key->key);
        if (  ret < 0 )
        {
            bMachinGetMacAddressStr(buff);
            sprintf(mesg, "invalide license key.[%s]", buff);
            return -1;
        }
        else if ( ret > 0)
        {
            sprintf(mesg, "failed to get system key.");
            return -1;
        }
        else
        {
            sprintf(mesg, "valide license key.");
            return 0;
        };

        break;
    case '1' : /* chaec date for 30 day */
        ret=PkgLicVelifyDemoData(key->key);
        if ( ret <= 0 )
        {
            sprintf(mesg, "demo license is expired");
            return -1;
        }
        else if ( ret > 30 )
        {
            sprintf(mesg, "date is too far");
            return -1;
        }
        else
        {
            sprintf(mesg, "%d day(s) remained.", ret);
            return 0;
        };

        break;

    default :
        sprintf(mesg, "unknown license version.");
        return -1;
    }

    return 0;
}

/*
MAC : "ABCDEF012345"
*/
int PkgLicVelifyMacAddr(char *in_mac)
{
    int ret, i;
    char this_mac[20], lic_mac[20];


    if ( strlen(in_mac) != 12)
    {
        return -1;
    };

    strcpy(lic_mac, in_mac);

    ret = bMachinGetMacAddressStr(this_mac);
    if (ret < 0)
    {
        /* failed to get mac address */
        return 1;
    };


    for (i=0; i<12; i++)
    {
        lic_mac[i] = toupper(lic_mac[i]);
        this_mac[i] = toupper(this_mac[i]);
    };

    /*
    printf("--- [%s]\n", lic_mac);
    printf("--- [%s]\n", this_mac);
    */
    if (strncmp(lic_mac, this_mac, 12) != 0)
    {
        /* invalid license key */
        return -1;
    };

    return 0;
}

/*
DATE : "20051130"
*/
int PkgLicVelifyDemoData(char *date)
{
    char buff[20];
    long curSec, inSec, deadSec, remainSec, remainDay;
    long sec30day = (30 * 24 * 60 * 60);
    long sec1day = (24 * 60 * 60);

    curSec = bGet_CurSecond();


    strcpy(buff, date);
    strcat(buff+8, "000000"); /* add time hh:mi:ss "00:00:00"*/

    inSec = bGet_SecFromTimeFmt(buff);
    deadSec = inSec + sec30day;

    remainSec = (deadSec - curSec);
    remainDay = (long)(remainSec / sec1day);

    printf("Trial-period remained [%d] day(s).\n", (int)remainDay)	;

    return (int)(remainDay);
}
