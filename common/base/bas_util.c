/* ==========================================
 *  File Name   :
 *  Author      :
 *  Date        :
 *  Comment     :
 * ========================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <ctype.h>


/* ===============================
 * unsigned char ConvertIntStringToByte(char *strInt)
 * "37" <-- 0x37
 * =============================== */
int bUtilConvNibbleToDigit(unsigned char *dest, unsigned char ch)
{
    unsigned char tmp;

    tmp = ch;
    tmp = tmp >> 4;
    dest[0] = tmp + '0';

    tmp = ch;
    tmp &= 0x0F;
    dest[1] = tmp + '0';

    return 0;
}

/* ===============================
 * unsigned char ConvertIntStringToByte(char *strInt)
 * "37" <-- 0x37
 * =============================== */
int bUtilConvByteToTwoDigit(char *dest, unsigned char ch)
{
    sprintf(dest, "%02d", (int)ch);

    return 0;
}

/* ===============================
 * unsigned char ConvertIntStringToByte(char *strInt)
 * 0x0A <- "10", 0xFF <- "255"
 * =============================== */
unsigned short bUtilConvNToH2(unsigned char *netByte)
{
    unsigned short  val;

    memcpy((unsigned char*)&val, netByte, 2);
    return ntohs(val);
}

unsigned long bUtilConvNToH4(unsigned char *netByte)
{
    unsigned long  val;

    memcpy((unsigned char*)&val, netByte, 4);
    return ntohl(val);
}

unsigned short bUtilConvHToN2(unsigned char *netByte)
{
    unsigned short  val;

    memcpy((unsigned char*)&val, netByte, 2);
    return htons(val);
}

unsigned long bUtilConvHToN4(unsigned char *netByte)
{
    unsigned long  val;

    memcpy((unsigned char*)&val, netByte, 4);
    return htonl(val);
}

/* ===============================
 * unsigned char ConvertIntStringToByte(char *strInt)
 * 0x0A <- "10", 0xFF <- "255"
 * =============================== */
unsigned char bUtilConvIntStringToByte(char *strInt)
{
    int val = atoi(strInt);
    return (unsigned char) val;
}

/* ===============================
 * int bUtilConvIPStrToByte4(unsigned char *desc, char *strIP)
 * 0x0A00FF0A <- "10.0.256.10"
 * =============================== */
int bUtilConvIPStrToByte4(unsigned char *dest, char *strIP)
{
    int i, len, cnt;
    char buff[40], *ptr;

    strcpy(buff, strIP);

    len = strlen(buff);
    cnt = 0;
    ptr = buff;
    for (i=0; i<=len; i++)
    {
        if ( ! isdigit(buff[i]) )
        {
            buff[i] = '\0';
            dest[cnt++] = bUtilConvIntStringToByte(ptr);
            if (cnt > 4)
            {
                break;
            };
            ptr = &buff[i+1];
        };
    }

    return 0;
}


