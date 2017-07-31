#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>



/* ==============================================================================
 * bFile_fdopen
 * file pointer OR NULL
 * ============================================================================== */
FILE * bFile_fdopen(int fildes, const char *mode)
{
    return fdopen(fildes, mode);
}



/* ==============================================================================
 * lock_fd
 * ============================================================================== */
int bLock_fd(int fd)
{
    struct flock lock;
    int ret;

    /* file locking */
    lock.l_type = F_WRLCK;
    lock.l_whence =  0;
    lock.l_start = 0;
    lock.l_len = 0;     /* entire file */

    if ( (ret=fcntl(fd, F_SETLKW, &lock)) < 0)
        perror("lock_fd");

    return ret;
}

/* ==============================================================================
 * lock_fp
 * ============================================================================== */
int bLock_fp(FILE *fp)
{
    return bLock_fd(fileno(fp));
}

/* ==============================================================================
 * unlock_fd
 * ============================================================================== */
int bUnlock_fd(int fd)
{

    struct flock lock;
    int ret;


    /* file unlock */
    lock.l_type = F_UNLCK;
    lock.l_whence =  0;
    lock.l_start = 0;
    lock.l_len = 0;

    if ( (ret=fcntl(fd, F_SETLKW, &lock)) < 0)
        perror("unlock_fd");

    return ret;
}

/* ==============================================================================
 * unlock_fp
 * ============================================================================== */
int bUnlock_fp(FILE *fp)
{
    return bUnlock_fd(fileno(fp));
}


/* ==============================================================================
 * writefd
 * ============================================================================== */
int bWritefd(int fd, char *s, ...)
{
    char	format[1024], buff[2048];
    va_list		ap;
    int ret;

    if (fd<0) return -1;

    sprintf(format, "%s", s);

    va_start(ap, s);
    vsprintf(buff, format, ap);
    va_end(ap);

    if (strlen(buff) > sizeof(buff)) return -1;

    lseek(fd, 0L, SEEK_END);
    if ( (ret=write(fd, buff, strlen(buff))) < 0)
        perror("writefd");

    return ret;
}



/* ==============================================================================
 * Is File Exist?
 * 1 is exist, 0 is not exist
 * ============================================================================== */
int bIs_there_file(char *filename)
{
    /* if filename = NULL */
    if (filename==NULL)
        return -1;

    if (access(filename, F_OK) < 0)
        return -1;

    return 1;
}

/* =============================== */




/*
Program 4.3 r_read.c
The r_read.c function is similar to read except that it restarts itself if interrupted by a signal.

#include <errno.h>
#include <unistd.h>
*/

ssize_t r_read(int fd, void *buf, size_t size)
{
    ssize_t retval;

    while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
    return retval;
}

/*
Program 4.4 r_write.c
The r_write.c function is similar to write except that it restarts itself if interrupted by a signal and writes the full amount requested.

#include <errno.h>
#include <unistd.h>
*/

ssize_t r_write(int fd, void *buf, size_t size)
{
    char *bufp;
    size_t bytestowrite;
    ssize_t byteswritten;
    size_t totalbytes;

    for (bufp = buf, bytestowrite = size, totalbytes = 0;
            bytestowrite > 0;
            bufp += byteswritten, bytestowrite -= byteswritten)
    {
        byteswritten = write(fd, bufp, bytestowrite);
        if ((byteswritten) == -1 && (errno != EINTR))
            return -1;
        if (byteswritten == -1)
            byteswritten = 0;
        totalbytes += byteswritten;
    }
    return totalbytes;
}

