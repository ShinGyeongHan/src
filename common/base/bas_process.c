#include <sys/types.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/procfs.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <string.h>

#ifdef _SOLARIS_
#include <sys/proc.h>
#endif

/* ============================
 * bDeamonise_this
 * ============================ */
pid_t bDeamonise_this( void (*sigHandler)())
{
    pid_t pid;

    pid = fork();

    if ( pid < 0 )
    {
        perror("bDeamonise_this");
        /*
        return (-1);
        */
        exit(0);
    };

    if ( pid != 0 )
    {
        exit(0);
    };

    /* become session leader */
    setsid();

    /* change working directory */
    chdir("/");

    /* clear mask */
    umask(0);

    /* signal processing
    signal (SIGHUP,  SIG_IGN);		// 제어단말기와 연결이 끊어졌을때
    signal (SIGINT,  sigHandler);	// Ctrl+C 인터럽트 키가 입력될 때
    signal (SIGQUIT, sigHandler);	// Ctrl+\ 종료키기 입력될때
    signal (SIGTSTP, sigHandler);	// Ctrl+Z suspend 키가 입력될때
    signal (SIGTERM, sigHandler);	// TERM signal에 의한 child종료
    signal (SIGILL,  sigHandler);	// 비정상적인 명령이 수행되려 할때
    signal (SIGUSR1, sigHandler);	// USER signal에 의한 강제 종료
    signal (SIGKILL, sigHandler);	// KILL signal에 의한 강제 종료
    */

    return pid;
}


/* ==============================================================================
 * bIs_there_process_pid
 * switch((kill(pid, 0))
 *	0 : there is process
 *	* : there is NO process
 * ============================================================================== */
int bIs_there_process_pid(int pid)
{
    return kill(pid, 0);
}



#ifdef _SOLARIS_
/*
 * is_pname_exist
 *
 * read process table and search pname
 * return pid On pname is exist, return 0 On pname is not exist
 */
int bIs_there_process_pname(char *pname)
{
    DIR  *dp;
    struct dirent *dirp;
    prpsinfo_t retval;
    char name[512];
    int fd;
    int ourretval = 0;

    if ((dp = opendir("/proc")) == NULL)
    {
        return -1;
    }

    while ((dirp = readdir(dp)) != NULL)
    {

        if (dirp->d_name[0] != '.')
        {

            snprintf(name, 512, "%s/%s", "/proc", dirp->d_name);

            if ((fd = open(name,O_RDONLY)) != -1)
            {

                if (ioctl(fd, PIOCPSINFO, &retval) != -1)
                {

                    if (!strcmp(retval.pr_fname, pname))
                    {
                        /* ourretval = (pid_t)atoi(dirp->d_name); */
                        ourretval++;
                        /* close(fd); */
                        /* break; */
                    }

                }
                close(fd);
            }
        }
    }
    closedir(dp);
    if (ourretval > 1) return 1;
    else return 0;
}
#endif


#ifdef _SOLARIS_
/*
 * pname_count
 *
 * read process table and search pname
 * return pid On pname is exist, return 0 On pname is not exist
 */
int _SOLARIS_bGet_pid_of_pname(char *pname)
{
    DIR  *dp;
    struct dirent *dirp;
    prpsinfo_t retval;
    char name[512];
    int fd;
    int ourretval = 0;

    if ((dp = opendir("/proc")) == NULL)
    {
        return -1;
    }

    while ((dirp = readdir(dp)) != NULL)
    {

        if (dirp->d_name[0] != '.')
        {

            snprintf(name, 512, "%s/%s", "/proc", dirp->d_name);

            if ((fd = open(name,O_RDONLY)) != -1)
            {

                if (ioctl(fd, PIOCPSINFO, &retval) != -1)
                {

                    if (!strcmp(retval.pr_fname, pname))
                    {
                        /* ourretval = (pid_t)atoi(dirp->d_name); */
                        ourretval++;
                        /* close(fd); */
                        /* break; */
                    }

                }
                close(fd);
            }
        }
    }
    closedir(dp);
    return ourretval;
}
#endif



/* ==============================================================================
 * get_first_line_from_file
 * ============================================================================== */
int bGet_first_line_from_file(char *fname, char *dest)
{
    FILE *fp;
    int ret;

    if ( NULL == (fp = fopen(fname, "r")) )
    {
        perror(fname);
        return -1;
    };

    if ( fgets( dest, 1024, fp) )
    {
        if ( dest [ strlen(dest) ] == '\n' )
        {
            dest [ strlen(dest)-1 ] = '\0';
        }
        else
        {
            dest [ strlen(dest) ] = '\0';
        }
        ret = 0;
    }
    else
    {
        perror("fgets");
        ret = -1;
    };

    fclose(fp);

    return ret;
}


/* ==============================================================================
 * get_pid_from_file
 * ============================================================================== */
int bGet_pid_from_file(char *fname_pid)
{
    char buff[1024];
    int  pid;

    bGet_first_line_from_file(fname_pid, buff);
    pid = ((pid=atoi(buff)) > 0)? pid : -1;

    return pid;
}

/* ==============================================================================
 * put_pid_to_file
 * ============================================================================== */
int bPut_pid_to_file(char *fname_pid, int pid)
{
    FILE *fp;

    if ( NULL == (fp = fopen(fname_pid, "w")) )
    {
        perror(fname_pid);
        return -1;
    };
    
    fprintf(fp, "%d\n", pid);
    fclose(fp);

    return 0;
}

/* ==============================================================================
 * append_pid_to_file
 * ============================================================================== */
int bAppend_pid_to_file(char *fname_pid, int pid)
{
    FILE *fp;

    if ( NULL == (fp = fopen(fname_pid, "a")) )
    {
        perror(fname_pid);
        return -1;
    };

    fprintf(fp, "%d\n", pid);

    fclose(fp);

    return 0;
}

/* ============================================================================== */

