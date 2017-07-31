/*	$OpenBSD: strsep.c,v 1.3 1997/08/20 04:28:14 millert Exp $	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string.h>
#include <stdio.h>

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)strsep.c	8.1 (Berkeley) 6/4/93";
#else
static char *rcsid = "$OpenBSD: strsep.c,v 1.3 1997/08/20 04:28:14 millert Exp $";
#endif
#endif /* LIBC_SCCS and not lint */

#ifndef HAVE_STRSEP
/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char *strsep(stringp, delim)
register char **stringp;
register const char *delim;
{
    register char *s;
    register const char *spanp;
    register int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;)
    {
        c = *s++;
        spanp = delim;
        do
        {
            if ((sc = *spanp++) == c)
            {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        }
        while (sc != 0);
    }
    /* NOTREACHED */
}
#endif /* ! HAVE_STRSEP */


#include <ctype.h>
#include <stdlib.h>


/*
 ******************************************************************************
 *  1. 이  름 : 321.ltrim (왼쪽 공백 제거)
 *  2. 설  명 : str형의 문자열중 왼쪽 공백을 wprj
 *  3. 변  수 : 1) char * src  (I) : 변환대상
 *  4. RETURN : 1) char * src      : 변환결과
 ******************************************************************************
 */
char *ltrim( src )
char *src;
{
    /* 왼쪽 Skip */
    while (*src && (*src == ' ' || *src == '\t')) src++;

    return src;

}

/*
 ******************************************************************************
 *  1. 이  름 : 322.rtrim (오른쪽 공백제거)
 *  2. 설  명 : str형의 문자열중 오른쪽 공백을 없애고 NULL을 붙혀 Return
 *              이때 src는 변경하지 않고 temp buffer에 내용을 복사하여 변경한다.
 *  3. 변  수 : 1) char * src  (I) : 변환대상
 *  4. RETURN : 1) 정상처리시 : char * s (변환결과)
 *              2) 비정상시   : char * src(입력 값)
 ******************************************************************************
 */
/*
char *rtrim(char *src)
{
    char * e=0;
    char * p;
    char * s;

    if (*src)
    {
        p = s = NextBuf();

        while(*src)
        {
            if      (*src == '\t' && !e) e = p;
            else if (*src == ' '  && !e) e = p;
            else if (*src != ' '       ) e = 0;

            *p++ = *src++;
        }

        if (e) *e = 0x00;
        else   *p = 0x00;

        return s;
    }

    return src;
}
*/


/*
 ******************************************************************************
 *  1. 이  름 : 323.dtrim (좌우공백제거)
 *  2. 설  명 : str형의 문자열중 좌우 공백을 없애고 NULL을 붙혀 Return
 *              이때 src는 변경하지 않고 temp buffer에 내용을 복사하여 변경한다.
 *  3. 변  수 : 1) char * src  (I) : 변환대상
 *  4. RETURN : 1) char * src : 변환결과
 ******************************************************************************
 */
/*
char *dtrim(char *src)
{
     왼쪽 Skip
    while(*src && (*src == ' ' || *src == '\t')) src++;

     오른쪽 Trimming
    return rtrim(src);
}
*/


/*
 ******************************************************************************
 *  1. 이  름 : 324.trim (모든 공백 제거)
 *  2. 설  명 : str형의 문자열중 모든 공백을 제거
 *  3. 변  수 : 1) char * str  (I) : 변환대상
 *  4. RETURN : 1) char * rstr     : 변환결과
 ******************************************************************************
 */
char *trim(str)
char *str;
{
    int i=0, j=0, len=0;
    char	*rstr;
    char *tmp;

    rstr = (char *) calloc(sizeof(char), strlen(str));
    if (strlen(str) < 1) return (rstr);

    tmp = (char *)calloc(sizeof(tmp),len);

    for (i=0; i<len; i++)
    {
        if ((str[i] == ' ') || (str[i] == '\t')) continue;
        tmp[j] = str[i];
        j += 1;
    }

    strncpy(rstr+0, tmp+0, strlen(tmp));

    return (rstr);
}

/*
 ******************************************************************************
 *  1. 이  름 : 325.SubStr
 *  2. 설  명 : 문자열중의 일부를 추출
 *  3. 변  수 : 1) char * str  (I) : SOURCE 문자열
 *              2) int start   (I) : 시작번호(0부터)
 *              3) int len     (I) : 추출할 길이
 *  4. RETURN : 1) char * buf      : 변환결과
 ******************************************************************************
 */
/*
char * SubStr(str, start,  len)
    char *str; int start, len;
{
    char *p, *buf;

    p = buf = NextBuf();
    str += start;

    while(len--)  *p++ = *str++;
    *p = 0x00;

    return buf;
}
*/
/*
 ******************************************************************************
 *  1. 이  름 : 326.l_mid
 *  2. 설  명 : Left Mid String
 *  3. 변  수 : 1) char * str   (I) : SOURCE 문자열
 *              2) int pos      (I) : mid위치
 *              3) char * lm_str(O) : 변환결과
 *  4. RETURN : 1)  (0)  : 정상처리
 *              2)  (-1) : 비정상
 ******************************************************************************
 */
int	 l_mid(str,pos,lm_str)
char *str, *lm_str;
int	 pos;
{
    if (strlen(str) < 1) return( (-1));

    strncpy(lm_str,str,pos-1);

    return( (0));
}

/*
 ******************************************************************************
 *  1. 이  름 : 327.r_mid
 *  2. 설  명 : Right Mid String
 *  3. 변  수 : 1) char * str   (I) : SOURCE 문자열
 *              2) int pos      (I) : mid위치
 *              3) char * rm_str(O) : 변환결과
 *  4. RETURN : 1)  (0)  : 정상처리
 *              2)  (-1) : 비정상
 ******************************************************************************
 */
int	r_mid(str,pos,rm_str)
char *str, *rm_str;
int	 pos;
{
    int	 i=0;

    if (strlen(str) < 1) return( (-1));

    i=(strlen(str)-(pos-1));

    strncpy(rm_str,str+(pos-1),i);

    return( (0));
}

/*
 ******************************************************************************
 *  1. 이  름 : 328.r_mid
 *  2. 설  명 : Middle Mid String
 *  3. 변  수 : 1) char * str   (I) : SOURCE 문자열
 *              2) int pos1     (I) : 시작위치
 *              3) int pos2     (I) : 종료위치
 *              4) char * mm_str(O) : 변환결과
 *  4. RETURN : 1)  (0)  : 정상처리
 *              2)  (-1) : 비정상
 ******************************************************************************
 */
int	 m_mid(str,pos1,pos2,mm_str)
int	 pos1,pos2;
char *str, *mm_str;
{
    int	 i=0;

    if (strlen(str) < 1) return( (-1));

    i = (strlen(str)-1) - (pos2 -1);
    strncpy(mm_str,str+(pos1-1),i);

    return( (0));
}


/*
 * setBlank
 * buf내의 null뒤의 값들을 공백으로 바꾼다.
 */
void setBlank(char *buf, int len)
{
    while (*buf && len)
    {
        buf++;
        len--;
    }
    while (len--)         *buf++ = ' ';
}


/*
 * no space copy
 * src의　뒷부분의　공백을　제거하고　target에　copy함
 * src의 앞부분에 공백이 있으면 안됨
 */
int noscpy(char *target, char *src)
{
    int i = 0;

    while (*src != 0x20 && *src != 0x00)
    {
        *target = *src;
        src++;
        target++;
        i++;
    }
    *target = 0x00;

    return i;
}


/*
 * no space n copy
 * src의　뒷부분의　공백을　제거하고　target에　copy함
 * src의 앞부분에 공백이 있으면 안됨
 * src를 n개까지 탐색함
 */
int nosncpy(char *target, char *src, int n)
{
    int i = 0;

    while (i < n)
    {
        if (*src != 0x00 && *src != 0x20)
        {
            target[i] = *src;
        }
        else      /* is null or space */
        {
            break;
        }
        src++;
        i++;
    }
    target[i] = 0x00;

    return i;
}


/*
 * isEmpty
 * 인수가 Empty인지 확인
 * 변수의 길이만큼 확인한다.
 * 중간에 null이 있으면 그곳까지만 확인한다.
 */
int isEmpty(char *msg, int len)
{
    if (!len)
    {
        /*	len = lstrlen(msg); */
        len = strlen(msg);
    };

    while (len--)
    {
        if (*msg == 0x00)    return  (0);
        if (*msg != ' ')     return  (-1);
        msg++;
    }

    return  (0);
}


/*
 * isDigit
 * 인수가 모두 숫자인가?
 */
int isDigit(char *str)
{
    while (*str)
    {
        if (!isdigit((int)*str++))
            return  (-1);
    };

    return  (0);
}

/*
 * to_upper
 * lowercase --> uppercase
 */
int to_upper(char *src, char *dest)
{
    int i,size;

    size = strlen(src);
    for (i=0; i < size; i++)
    {
        dest[i] = toupper(*(src+i));
    }
    dest[i] = '\0';

    return  (0);
}

/*
 * replace_all
 * replace olds --> news
 */

char *replace_all(char *s, const char *olds, const char *news)
{
    char *result, *sr;
    size_t i, count = 0;
    size_t oldlen = strlen(olds);
    if (oldlen < 1) return s;
    size_t newlen = strlen(news);


    if (newlen != oldlen)
    {
        for (i = 0; s[i] != '\0';)
        {
            if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
            else i++;
        }
    }
    else i = strlen(s);


    result = (char *) malloc(i + 1 + count * (newlen - oldlen));
    if (result == NULL) return NULL;


    sr = result;
    while (*s)
    {
        if (memcmp(s, olds, oldlen) == 0)
        {
            memcpy(sr, news, newlen);
            sr += newlen;
            s  += oldlen;
        }
        else *sr++ = *s++;
    }
    *sr = '\0';

    return result;
}


/*
 *=============================================================================
 *  End of File
 *=============================================================================
 */
