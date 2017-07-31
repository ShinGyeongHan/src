#include <stdio.h>
#include <sys/time.h>
#include <signal.h>


void (*sigset(int sig, void (*disp)(int)))(int);

/* =======================
 * No use alarm signal
 * ======================= */
void bSleep(long sec, long usec)
{
    struct timeval tv;

    tv.tv_sec = sec;   /*second */
    tv.tv_usec = usec;    /* Microsecond  */
    select(0, NULL, NULL, NULL, &tv);
    /*
    주의 : select 가 호출하고 나면 tv의 값이 초기화 (리눅스 버전)
    */
}

/* ============================
 * bSet_timer
 * Timer & Handler (Send ARLM signal periodically.)
 * ============================ */
void bSet_timer (int sec, int usec, void (*handler)())
{
    struct itimerval itv;
    int ret;

    itv.it_interval.tv_sec = sec;
    itv.it_interval.tv_usec = usec;

    itv.it_value.tv_sec = sec;
    itv.it_value.tv_usec = usec;

    ret = setitimer(ITIMER_REAL, &itv, (struct itimerval *) 0);

    sigset(SIGALRM, handler);
}


void bUnSet_timer ()
{
    signal(SIGALRM, SIG_IGN);
}
