#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

char* bGet_date(char *dest, int strlen, char *format);

char *basename(const char *path);

static char fname_log[128];
static int  logDest;

/******************************************
 * INPUT :
 * 1. pname : program name used as base name of log file.
 * 2. log : log destination, can be mutiplex.
 *     0 = NO LOG
 *     1 = to TERMINAL
 *     2 = to FILE
 *     3 = to ALL
 * OUTPUT : n/a
 *****************************************/
void bStart_log(char *pname, int log)
{
    if ( log < 0 || log > 3)
    {
        printf("log destination = {0|1|2}\n");
        /*exit(0);*/
        logDest = 3;
    }
    else
    {
        logDest = log;
    };

    if ( logDest >= 2 )
    {
        sprintf(fname_log, "%s", pname);
    };
}


/******************************************
 * bLog
 *****************************************/
void bLog(char *fmt, ...)
{
    FILE    *fp;
    va_list vlst;
    char    buf[256], logfile[256];
    char    ebuf[2048];

    if ( logDest < 1 || logDest > 3)
    {
        return;
    };

    va_start(vlst, fmt);
    vsprintf(ebuf, fmt, vlst);
    va_end(vlst);

    strcat(ebuf, "\n");

    if ( logDest == 1 )
    {
        fprintf(stderr, ebuf);
        return;
    }
    else if ( logDest == 3 )
    {
        fprintf (stderr, ebuf);
    };

    sprintf(logfile, "%s", (char*)bGet_date(buf, sizeof(buf), fname_log));

    if (NULL == (fp=fopen(logfile, "a")))
    {
        perror(logfile);
    };


    fprintf(fp, "[%s] %s",
            (char*)bGet_date(buf, sizeof(buf), "%y/%m/%d-%H:%M:%S"),
            ebuf);

    fflush(fp);

    fclose(fp);
}



/******************************************
 * bLogDest
 * 'logDest' like as this 	"/tmp/logeg_%Y%m%d.log"
 *****************************************/
void bLogDest(char *logDest, char *fmt, ...)
{
    FILE    *fp;
    va_list vlst;
    char    buf[256], logfile[256];
    char    ebuf[2048];

    va_start(vlst, fmt);
    vsprintf(ebuf, fmt, vlst);
    va_end(vlst);
    strcat(ebuf, "\n");

    sprintf(logfile, "%s", (char*)bGet_date(buf, sizeof(buf), logDest));

    if (NULL == (fp=fopen(logfile, "a+")))
    {
        perror(logfile);
        return;
    };


    fprintf(fp, "[%s] %s",
            (char*)bGet_date(buf, sizeof(buf), "%y/%m/%d-%H:%M:%S"),
            ebuf);

    fflush(fp);

    fclose(fp);
}

/******************************************
 * bLog_hex
 *****************************************/
void bLog_hex(const char *title, const unsigned char *data, int size )
{
    int i, rowsize, offset;
    char *bufpos, buffer[128];	/* XXX handle buffer overflow */

    if ( logDest < 1 || logDest > 3)
    {
        return;
    };

    bLog("%s : size[%d]", title, size);

    for ( offset = 0; size;  )
    {
        rowsize = ( size > 16 ) ? 16 : size;
        sprintf( buffer, "%04x: ", offset );
        bufpos = buffer + strlen( buffer );

        for ( i = 0; i < rowsize; i++ )
        {
            sprintf( bufpos, "%02x ", data[i] );
            bufpos += 3;
        };

        for ( ; i < 16; i++ )
        {
            strcat( buffer, "   " );
        };

        strcat( buffer, "  " );
        bufpos = buffer + strlen( buffer );
        for ( i = 0; i < rowsize; i++, bufpos++ )
        {
            sprintf( bufpos, "%c", isprint( data[i] ) ? data[i] : '.' );
        };

        size -= rowsize;
        data += rowsize;
        offset += rowsize;

        bLog(buffer);
    }
}

/*
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <string.h>
#include <sys/param.h>

char *basename(const char *path)
{
    static char bname[MAXPATHLEN];
    const char *endp, *startp;

    /* Empty or NULL string gets treated as "." */
    if (path == NULL || *path == '\0')
    {
        (void)strcpy(bname, ".");
        return(bname);
    }

    /* Strip trailing slashes */
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/')
        endp--;

    /* All slashes becomes "/" */
    if (endp == path && *endp == '/')
    {
        (void)strcpy(bname, "/");
        return(bname);
    }

    /* Find the start of the base */
    startp = endp;
    while (startp > path && *(startp - 1) != '/')
        startp--;

    if (endp - startp + 2 > sizeof(bname))
    {
        errno = ENAMETOOLONG;
        return(NULL);
    }
    (void)strncpy(bname, startp, endp - startp + 1);
    bname[endp - startp + 1] = '\0';
    return(bname);
}
