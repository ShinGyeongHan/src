#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _LINUX_
#include <time.h>
#endif


/* ===============================
 * bGet_cur_tm
 * =============================== */
struct tm* bGet_cur_tm()
{
    time_t on_time;

    tzset ();                            /*  set timezone */;
    on_time = time(NULL);
    return (struct tm *)localtime(&on_time);
}

/* =============================================
*		int bGet_date_from_sec() ,
* format exam : "%Y/%m/%d/%H/%M/%S"
* ============================================= */
int bGet_date_from_sec(long sec, char *strdate, char *fmt)
{
    struct tm *tm, *localtime();

    tm = localtime(&sec);

#if (TICOM || MIPS || MIR_8800)
    return (ascftime (strdate, fmt, tm));
#else
    return (strftime(strdate, 40, fmt, tm));
#endif
}


/* ==========================================
 *     bGet_date_by_day
 * format exam : "%Y/%m/%d/%H/%M/%S"
 * ========================================== */
int bGet_date_by_day(char *date, int strlen, char *fmt, int day)
{
    long    tmToday, tmAnyday;

    tzset ();                            /*  set timezone */;

    time(&tmToday);
    tmAnyday = tmToday + ( (day) * 86400 ); // +-(day)*24hour

    bGet_date_from_sec(tmAnyday, date, fmt);

    return 0;
}

/* ===============================
 * bGet_date
 * format exam : "%Y/%m/%d/%H/%M/%S"
 * =============================== */
char* bGet_date(char *date, int strlen, char *fmt)
{
    bGet_date_by_day(date, strlen, fmt, 0); // today

    return date;
}


/* =============================================
 *		long SBaTimGetCurSecond()
 * ============================================= */
long bGet_CurSecond()
{
    long      curTime;

    time(&curTime);
    return curTime;
}

/* =============================================
 * 		int bCheck_IsValidDate()
 * ============================================= */
int bCheck_IsValidDate(int year, int month, int day)
{
    int dEnd;

    if (month < 1 || month > 12)
    {
        return -1;
    }

    switch (month)
    {
    case 1 :
        dEnd = 31;
        break;
    case 2 :
        if ((year % 4) == 0 || (year % 100) == 0)
            dEnd = 29;
        else
            dEnd = 28;
        break;
    case 3 :
        dEnd = 31;
        break;
    case 4 :
        dEnd = 30;
        break;
    case 5 :
        dEnd = 31;
        break;
    case 6 :
        dEnd = 30;
        break;
    case 7 :
        dEnd = 31;
        break;
    case 8 :
        dEnd = 31;
        break;
    case 9 :
        dEnd = 30;
        break;
    case 10 :
        dEnd = 31;
        break;
    case 11 :
        dEnd = 30;
        break;
    case 12 :
        dEnd = 31;
        break;
    }

    if (day < 1 || day > dEnd)
    {
        return -1;
    };

    return (0);
}

/* =============================================
 * 		long SBaTimGetSecFromTime()
 * format will be YYYYMMDDHHMISS.
 * Zero Time In Korea is 17700101090000
 * so caltime and minus (9 * 3600);
 * ============================================= */
long bGet_SecFromTimeFmt(char *stime)
{
    struct tm tm;
    char buff[8];

    int yy, mm, dd, hh, mi, ss;

    if (strlen(stime) < 14)
    {
        return -1;
    };


    sprintf(buff, "%s", stime);
    buff[4] = '\0';
    yy = atoi(buff);

    sprintf(buff, "%s", stime+4);
    buff[2] = '\0';
    mm = atoi(buff);

    sprintf(buff, "%s", stime+6);
    buff[2] = '\0';
    dd = atoi(buff);

    sprintf(buff, "%s", stime+8);
    buff[2] = '\0';
    hh = atoi(buff);

    sprintf(buff, "%s", stime+10);
    buff[2] = '\0';
    mi = atoi(buff);

    sprintf(buff, "%s", stime+12);
    buff[2] = '\0';
    ss = atoi(buff);


    if ( bCheck_IsValidDate(yy, mm, dd) < 0)
    {
        return -1;
    };

    tm.tm_year = yy - 1900;
    tm.tm_mon = mm-1;
    tm.tm_mday = dd;
    tm.tm_hour = hh;
    tm.tm_min = mi;
    tm.tm_sec = ss;
    tm.tm_isdst = 0;

    return mktime(&tm);
}




