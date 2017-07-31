#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifdef _SOLARIS_
#include <sys/filio.h>
#endif


/* ==========================================
 *		int bTCP_GetIPAddrByName()
 * ========================================== */
int bTCP_GetIPAddrByName(char* ipAddr, char* hostname)
{
    struct hostent* hent;
    struct in_addr *ptr;

    if ((hent = gethostbyname(hostname)) == NULL)
    {
        ipAddr[0] = '\0';
        printf( "Can't get hostid by name");
        return -1;
    }
    else if (hent->h_addrtype != AF_INET)
    {
        ipAddr[0] = '\0';
        printf("It's not AF_INET");
        return -1;
    }
    else
    {
        ptr = (struct in_addr*) hent->h_addr_list[0];
        strcpy(ipAddr, inet_ntoa(*ptr));
        return 0;
    }
}



/* ==========================================
 *		int bTCP_GetHostNameByIPAddr()
 * ========================================== */
int bTCP_GetHostNameByIPAddr(char* hostname, char* ipAddr)
{
    struct hostent* hent;
    u_long addr;

    if ((int)(addr = inet_addr(ipAddr)) == -1)
    {
        printf("Addr Format Should be A.B.C.D");
        return -1;
    }

    if ((hent = gethostbyaddr((char*)&addr,
                              sizeof(addr), AF_INET)) == NULL)
    {
        hostname[0] = '\0';
        return -1;
    }
    else
    {
        strcpy(hostname, hent->h_name);
        return 0;
    }
}


/* ====================================
 * bGet_server_socket
 * return: -1 on error, fd on success
 * ==================================== */
static int bGet_server_socket(int port, int listenq)
{
    struct sockaddr_in addr;
    int fd;
    int on = 1, keepFlag=1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("bGet_server_socket():socket[%s]\n", strerror(errno));
        return -1;
    };

    bzero((char*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(addr.sin_zero), 8);

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int *)&on, sizeof(on));

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                   (char*)&keepFlag, sizeof(int)) < 0)
    {
        printf("can't set keep alive");
    }



    if ( bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0 )
    {
        printf("bGet_server_socket():bind[%s], port[%d]\n", strerror(errno), port);
        return -1;
    };

    if ( listen(fd, listenq) < 0 )
    {
        printf("bGet_server_socket():listen[%s]\n", strerror(errno));
        return -1;
    };

    return fd;
}


/* ===============================
 * serv_init_server_sock
 * =============================== */
int bInit_server_socket(int port, int listenq)
{
    int fd;

    if ( port < 0 )
    {
        /*printf("bInit_server_socket():invalid port:[%d]\n", port);*/
        return -1;
    };

    if ( listenq < 0 )
    {
        listenq = 5;
    };


    fd = bGet_server_socket(port, listenq);

    if ( fd < 0 )
    {
        /*printf("bInit_server_socket():fail to open server socket.\n");*/
        return -1;
    };

    return fd;

}

/* ===============================
 * bDo_serve_clients  forever
 * =============================== */
void bDo_serve_clients(int listenfd,  void *(*CallBackFunc)())
{
    struct	sockaddr_in cliaddr;
    unsigned int     clilen=0;
    pid_t	childpid;
    int	connfd;

    for (;;)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

        if ( (childpid = fork()) == 0 )
        {
            close(listenfd);
            CallBackFunc(connfd);
            exit(0);
        };

        close(connfd);
    }
}



/* ===============================
 * bDo_serve_forever
 * =============================== */
void bDo_serve(int server,  void *(*CallBack_read_socket)())
{
    struct	sockaddr_in addr;
    unsigned int     addrlen=0;
    static  fd_set fdReadSet, fdTmpSet;
    static	int fdMax;

    int fd;
    int nread;
    int ret;


    /* 파일기슬자 집합(READ)에 서버소켓 추가 */
    FD_SET(server, &fdReadSet);
    fdMax = server;

    while (1)
    {

        fdTmpSet = fdReadSet;

        ret = select ( fdMax + 1, &fdTmpSet, (fd_set *)0, (fd_set*)0, (struct timeval*)0);
        /* 
           printf("select=[%d]\n", ret);
        */
        if (ret < 1)
        {
            /*
            perror("select");
            */
            continue;
        };

        /* 서버의 활성화 즉, 새로운 클라이언트의 연결 요청 */
        if ( FD_ISSET(server, &fdTmpSet))
        {
            fd = accept(server,(struct sockaddr *)&(addr), &addrlen);
            if (fd == -1)
            {
                perror("accept");
                break;
            };

            /* 소켓을 RW_fdset에 추가 */
            FD_SET(fd, &fdReadSet);
            printf("adding NEW client fd [%d]\n", fd);

            fdMax = (fdMax < fd)? fd : fdMax;

            continue;
        };


        /* 기존 클라이언트의 활성화 즉, 메시지 입력*/
        for (fd=4; fd<fdMax+1; fd++ )
        {
            if ( !(FD_ISSET(fd, &fdTmpSet)) )
            {
                continue;
            };

            printf("read from fd[%d]\n", fd);

            /* get bytesize to read from fd */
            ioctl(fd, FIONREAD, &nread);

            if (nread == 0) /* 클라이언트의 종료 요청 */
            {
                FD_CLR(fd, &fdReadSet);
                close(fd);
                printf("removing fd[%d]\n", fd);

                fdMax = (fdMax == fd)? fdMax-1 : fdMax;
            }
            else /* 클라이언트로 부터  메시지 읽기*/
            {
                CallBack_read_socket(fd);
            };
        };
    }; /*while*/
}




/*
 * read n bytes
 * return: -1 on error, length on success
 */
int bRead_nbyte(int fd, void *vptr, int nbytes)
{
    int	nleft, nread;
    char *ptr;

    ptr = vptr;
    nleft = nbytes;

    while (nleft > 0)
    {

        if ((nread = read(fd, ptr, nleft)) < 0 )
        {
            if (errno == EINTR)
                nread = 0;		/* and call read() again */
            else
                return -1;
        }
        else if (nread == 0)
        {
            break;	/* EOF */
        };

        nleft -= nread;
        ptr   += nread;
    };

    return(nbytes - nleft);		/* return >= 0 */
}


/*
 * write n bytes
 * return: -1 on error, length on success
 */
int bWrite_nbyte(int fd, const void *vptr, int nbytes)
{
    int	nleft, nwritten;
    char *ptr;

    ptr = (char *)vptr;
    nleft = nbytes;

    while (nleft > 0)
    {

        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (errno == EINTR)
            {
                printf("[writen]write EINTR error\n");
                nwritten = 0;		/* and call write() again */
            }
            else
            {
                return -1;          /* error */
            };
        };

        nleft -= nwritten;
        ptr   += nwritten;
    };

    return(nbytes - nleft);
}

