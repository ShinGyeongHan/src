#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef _SOLARIS_
#include <sys/termio.h>
#endif

int kbhit()
{
    struct termio t;
    unsigned short flag;
    unsigned char min, time;
    char buf[10];
    ioctl(0, TCGETA, &t);    /* 표준입력 상태파악 */

    flag= t.c_lflag;         /* 값 변경 */
    min= t.c_cc[VMIN];
    time= t.c_cc[VTIME];

    t.c_lflag &= ~ICANON;     /* low 모드로 설정 */
    t.c_cc[VMIN] = 0;         /* read호출시 0개문자 읽어들임 */
    t.c_cc[VTIME]= 0;         /* 시간지연 없음 */

    ioctl(0, TCSETA, &t);     /* 상태변경 */

    if (read(0, buf, 9) <=  0)  /* read호출 */
    {
        t.c_lflag = flag;      /* 원상태로 복구 */
        t.c_cc[VMIN] = min;
        t.c_cc[VTIME]= time;
        ioctl(0, TCSETA, &t);
        return 0;             /*키가 안눌러졌음 */
    }
    else
    {
        t.c_lflag = flag;     /* 원상태로 복구 */
        t.c_cc[VMIN]= min;
        t.c_cc[VTIME]= time;
        ioctl(0, TCSETA, &t);

        return 1;             /* 키가 눌러졌음을 알림 */
    };
}

/* kbhit 함수 테스트 -- 임의의 키를 누르면 즉시 끝난다.
void main() {
   while(kbhit() == 0)
      printf("Welcome to Linux World\n");
   exit(0);
}

*/

