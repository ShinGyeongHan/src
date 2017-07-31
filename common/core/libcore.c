#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <iconv.h>

#include "sha1.h"
#include "common.h"


/* Global Conf */
SysInfo sysInfo;


/******************************************
 * LogPrint
 * 'U' logkind
 *                   -1 : Absolute terminal print
 *****************************************/
int gettimeofday();
int microtime(double *msec){
	
	struct timeval tp;
	
    if(gettimeofday((struct timeval *)&tp,NULL)==0){
        (*msec)=(double)(tp.tv_usec/1000000.00);
        if((*msec)>=1.0) (*msec)-=(long)(*msec);
        return 0;
    }else{
        return 1;
    }
}

void LogPrint(int logKind, char *logDest, char *fmt, ...)
{
	FILE*	fp;
	time_t	pt;
	struct	tm	lt; 
	double msec = 0;
	
	char folder[100];
	char LogFile[256];
	char *_logFile;
	DIR*    dir; 
		
	char logbuf[LOG_PACKET_LIMIT];
		
	va_list vlst;

    va_start(vlst, fmt);
    vsprintf(logbuf, fmt, vlst);
    va_end(vlst);
    //strcat(logbuf, "\n");
    
	time(&pt);
	localtime_r(&pt, &lt);
	microtime(&msec);
	
	if(logKind == SQL_DEBUG)
	{
		
		sprintf(folder, "%s", DAEMON_LOG);
		dir = opendir(folder);
		if(!dir) mkdir(folder, 0755);
		else closedir(dir);
		
		sprintf(LogFile, "%s/%s", folder, DAEMON_SQL_FILE);
		
		fp = fopen(LogFile,"a");
		if(fp)
		{
			fprintf(fp, "%s\n", logbuf); 
			fflush(fp);
			fclose(fp);
		};		
		
		
	}
	else if(logKind == FAIL || logKind == DEBUG)
	{
			
		sprintf(folder, "%s", DAEMON_LOG);
		dir = opendir(folder);
		if(!dir) mkdir(folder, 0755);
		else closedir(dir);
		
		sprintf(LogFile, "%s/%s", folder, DAEMON_ERROR_FILE);
		fp = fopen(LogFile,"a");
		if(fp)
		{
			fprintf(fp, "[%04d/%02d/%02d %02d:%02d:%02d (%8f)] %s\n", lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, msec, logbuf);
			fflush(fp);
			fclose(fp);
		};
		
    }
    else
    {    	
		sprintf(folder, "%s/%04d%02d", DAEMON_LOG, lt.tm_year+1900, lt.tm_mon+1);
		dir = opendir(folder);
		if(!dir) mkdir(folder, 0755);
		else closedir(dir);
		
		sprintf(folder, "%s/%d", folder, lt.tm_mday);
		dir = opendir(folder);
		if(!dir) mkdir(folder, 0755);
		else closedir(dir);
			
		sprintf(LogFile, "%s/%s.log", folder, logDest);	
		fp = fopen(LogFile,"a");
		if(fp)
		{
			fprintf(fp, "[%04d/%02d/%02d %02d:%02d:%02d (%8f)] %s\n", lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, msec, logbuf);
			fflush(fp);
			fclose(fp);
		};
	};
	
	
	if(logKind == SHOW) fprintf(stderr, "[%04d/%02d/%02d %02d:%02d:%02d (%8f)] %s\n", lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, msec, logbuf);
	
}


/*********************************************
STANDARD Configure_Load Function
*********************************************/
int Configuration()
{	
	char line[1024];
	char name[100], value[100];	
	FILE* fp;
	
    memset(&sysInfo, 0x00, sizeof(sysInfo)); 
    
    fp = fopen(CONF_FILE,"r"); 
	if(!fp) return FAIL;

	while(fgets(line, sizeof(line), fp))
	{

		if(line[0] == '#') continue;	
		if(strlen(line) < 5) continue;

		sscanf(line, "%s%s", name, value);

		if(strcmp(name, "DEBUGMODE") == 0)						sysInfo.DEBUGMODE = atoi(value);
		else if(strcmp(name, "LOG_RECIVE_PORT") == 0)			sysInfo.LOG_RECIVE_PORT = atoi(value);
		else if(strcmp(name, "RTS_SERVER_IP") == 0)				strcpy(sysInfo.RTS_SERVER_IP,value);
		else if(strcmp(name, "RTS_SERVER_PORT") == 0)			sysInfo.RTS_SERVER_PORT = atoi(value);
		else if(strcmp(name, "RTS_REPORT1_PORT") == 0)			sysInfo.RTS_REPORT1_PORT = atoi(value);
		else if(strcmp(name, "RTS_REPORT2_PORT") == 0)			sysInfo.RTS_REPORT2_PORT = atoi(value);
		else if(strcmp(name, "RTS_REPORT3_PORT") == 0)			sysInfo.RTS_REPORT3_PORT = atoi(value);
		else if(strcmp(name, "RTS_REPORT4_PORT") == 0)			sysInfo.RTS_REPORT4_PORT = atoi(value);
		else if(strcmp(name, "RTS_REPORT5_PORT") == 0)			sysInfo.RTS_REPORT5_PORT = atoi(value);

			
	};		
	fclose(fp);
    
	return SUCCESS;	
}

/* ================================================================================================
 * int SystemCommandExcute(char *cmd, char *result)
 *  - OS???? ???? ??????? ???
 *    . cmd    : ????????? ??? ????  
 *    . result : ???? ???
 * ============================================================================================= */
int SystemCommandExcute(char *cmd, char *result)
{
    char cmdBuff[256];
    FILE *fp;

    memset((char*)&cmdBuff, '\0', 256);

    sprintf(cmdBuff, "%s", cmd);

    if ((fp = popen(cmdBuff, "r")) == NULL)
    {
        return -1;              /* Failure to open the pipe */
    }

    memset((char*)&cmdBuff, '\0', 256);
    fread(cmdBuff, 1, 256, fp);
    cmdBuff[strlen(cmdBuff)] = '\0';

    if (!strncmp("\0", cmdBuff, 256))
    {
        pclose(fp);
        return -1;              /* fread  failed */
    }
    strcpy(result, cmdBuff);
    result[strlen(result)] = '\0';
    
    pclose(fp);
    
    return 0;
}


/* ================================================================================================
 checkTime(char *result)
 *  - ??? ????? ????? ??? ???
 *    . result : ???? ???
 * ============================================================================================= */
void checkTime(char *result)
{
	time_t	pt;
	struct	tm	lt;
    char cmdBuff[256];
    double msec = 0;

    memset((char*)&cmdBuff, '\0', 256);

	//////////////////
	/*	??? ??	*/
	time(&pt);
	localtime_r(&pt, &lt);
	microtime(&msec);
	
	sprintf(cmdBuff, "%04d-%02d-%02d %02d:%02d:%02d (%8f)", lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, msec);
    strcpy(result, cmdBuff);
    result[strlen(result)] = '\0';
}



/*********************************************/
// Make Time
/*********************************************/
time_t MakeTime(char *date)
{
	char* p;
	char tmp[20];
	int yy, mm, dd;
	struct tm   tm;
	
	strcpy(tmp, date);
	p = tmp;
	
	while(*p) {
		if(*p == '-')   *p = ' ';
		p++;
	}
		
	sscanf(tmp, "%d %d %d", &yy, &mm, &dd);
	
	memset(&tm, 0, sizeof(struct tm));
	tm.tm_year = yy-1900;
	tm.tm_mon = mm-1;
	tm.tm_mday = dd;

	return mktime(&tm);
}


/*********************************************/
// String space Delete Method
/*********************************************/
char* DeleteBlank(char *input)
{
	char buffer[1024];
	char *p, *q;

	p = input;
	q = buffer;
	while( *p != '\0' ) {
		if( !isspace(*p) ) {
			*(q++) = *(p++);
		}else {
			p++;
		}
	}

	*q = '\0';
	strcpy( input, buffer );
}


/* ================================================================================================
 * char *subString(char *str, int start, int len)
 *  - ????? ????????? ??? ????? ???? ??????? ???
 *  - Input Parameter
 *    . str    : ????? ??  
 *    . start  : ???? position
 *    . len    : ???? ????? ????
 * ============================================================================================= */
char *subString(char *str, int start, int len)
{
    int end;

    end = start + len;

    char *result = (char *) malloc (1 + end - start);

    strncpy (result, str + start, end - start);
    result[end - start] = '\0';

    return (result);
}


/* ================================================================================================
 * FileSizeGet(char* file)
 *  - ????????? ????? ???
 *  - Input Parameter 
 *    . file    : ???+?????
 * ============================================================================================= */ 
int FileSizeGet(char* file)
{
	int size = 0;
	FILE *fp;
	
	fp = fopen(file, "rb");
	if(fp)
	{
		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose(fp);
	};
	return size;
	
};

/* ================================================================================================
 * roundToInt(double x)
 * ============================================================================================= */ 
int roundToInt(double x) {
	if (x >= 0) return (int) (x + 0.5);
	return (int) (x - 0.5);
}


/* ================================================================================================
 * itoa
 *  - int cast string
 * ============================================================================================= */
void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

 void itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

/* ================================================================================================
 * FileSizeGet_KB(char* file)
 *  - ????????? ????? ???, 0.0KB ?? ???? ???
 *  - Input Parameter
 *    . file    : ???+?????
 * ============================================================================================= */ 
double FileSizeGet_KB(char* file)
{
	double temp;
	int size = 0;
	int byte = 1024*1024;
	FILE *fp;
	
	
	fp = fopen(file, "rb");
	if(fp)
	{
		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose(fp);
	};
		
	temp = (double) size/byte;
	temp = temp * pow(10, 1);// ????? ????? ???????? 10?? ?????? ??
	temp = floor( temp + 0.5 );// 0.5?? ?????? ??????? ?????? ??
	temp *= pow( 10, -1 );// ??? ?? ????? ???????
	
	return temp;

};


/* ================================================================================================
 * char* string_upper(char *src)
 *  - ?????? ????? ???????? ???
 *  - Input Parameter
 *    . src    : ?????
 * ============================================================================================= */
char* string_upper(char *src)
{
	while(*src != '\0') {
		*src = toupper(*src);
		src++;
	}
	return src;
}


/* ================================================================================================
 * char* string_lower(char *src)
 *  - ????? ?????? ???????? ???
 *  - Input Parameter
 *    . src    : ?????
 * ============================================================================================= */
char* string_lower(char *src)
{
	while(*src != '\0') {
		*src = tolower(*src);
		src++;
	}
	return src;
}


/* ================================================================================================
 * void replace(char* target, char* oldstr, char* newstr)
 *  - ????? ????????? ????? ???
 *  - Input Parameter
 *    . target    : ????? ??  
 *    . oldstr    : ????? ????
 *    . newstr    : ????? ????
 * ============================================================================================= */
char* search(char* target, char* find)
{
  int s1, s2;
  char* p;

  s1 = strlen(target);
  s2 = strlen(find);

  for(p=target; p<=target+s1-s2; p++) {
    if(strncmp(p, find, s2) == 0)
      return p;
  }
  return NULL;
}
void replace(char* target, char* oldstr, char* newstr)
{
  char *p;
  char tmp[1024*20];

  p = search(target, oldstr);
  while(p != NULL) {
    *p = '\0';
    strcpy(tmp, p+strlen(oldstr));
    strcat(target, newstr);
    strcat(target, tmp);
    p = search(p+ strlen(newstr), oldstr);
  }
}
/* ================================================================================================
 * int urldecode(unsigned char *source, unsigned char *dest) 
 *  - URL Decoding
 * ============================================================================================= */
static char _x2c(char hex_up, char hex_low) 
{
        char digit;

        digit = 16 * (hex_up >= 'A' 
                ? ((hex_up & 0xdf) - 'A') + 10 : (hex_up - '0'));
        digit += (hex_low >= 'A' 
                ? ((hex_low & 0xdf) - 'A') + 10 : (hex_low - '0'));
        return (digit);
}

size_t qURLdecode(char *str) 
{
	if (str == NULL) {
		return 0;
	}
	
	char *pEncPt, *pBinPt = str;
	for(pEncPt = str; *pEncPt != '\0'; pEncPt++) {
		switch (*pEncPt) {
			case '+': {
				*pBinPt++ = ' ';
				break;
			}
			case '%': {
				*pBinPt++ = _x2c(*(pEncPt + 1), *(pEncPt + 2));
				pEncPt += 2;
				break;
			}
			default: {
				*pBinPt++ = *pEncPt;
				break;
			}
		}
	}
	*pBinPt = '\0';
	
	return (pBinPt - str);
}


/* ================================================================================================
 * int killAllPidFile(char *fname_pid)
 *  - Input Parameter
 *    . fname_pid    : Process ID ????????. ??) /var/run/~.pid ??  
 * ============================================================================================= */
int killAllPidFile(char *fname_pid)
{
    char buff[1024];
    int  pid;

    FILE *fp;
    int ret = -1;

    if ( NULL == (fp = fopen(fname_pid, "r")) )
    {
        perror(fname_pid);
        return -1;
    };

    while ( fgets( buff, 1024, fp) != NULL)
    {
        if ( buff [ strlen(buff) ] == '\n' )
        {
            buff [ strlen(buff)-1 ] = '\0';
        }
        else
        {
            buff [ strlen(buff) ] = '\0';
        }
        pid = ((pid=atoi(buff)) > 0)? pid : -1;
        ret = kill(pid, SIGTERM);
    }

    fclose(fp);
    return ret;
}



/*********************************************/
//????? ????? ????????? ??????? ???? ???
//????? ???? ??????, ????????? ???? ????
/*********************************************/
char *strstr_h(const char *s1, const char *s2)
{
    unsigned char *p1, *p2;

	p1=(unsigned char *)s1; 
	p2=(unsigned char *)s2;

	while(*p1)
	{
        if ((*p1 >= 0xa0) && (*p2 >= 0xa0))
		{
            if ((*p1==*p2) && (*(p1+1)==*(p2+1)))
			{
                p1+=2;	p2+=2;
			}else{
                p1+=2;  p2=(unsigned char *)s2;
			}
		}else if ((*p1 < 0xa0) && (*p2 < 0xa0))
		{
			if (*p1==*p2) 
			{
				p1++;   p2++;
			}else{
                p1++;   p2=(unsigned char *)s2;
			}
		}else{
			if (*p1 >= 0xa0) p1+=2; else p1++;
            p2=(unsigned char *)s2;
		}
		
		if (*p2=='\0')
		{
             return p2;
		}
	}

    return NULL;
}


/*********************************************/
//?????? ???????? ????? ???
/*********************************************/
char *strcasestr(char *szSrc, char *szFind )
{
	int i, k, nFind, nSrc;

	if( !szSrc || !szSrc[0] || !szFind || !szFind[0] )  { return(0); }

	nSrc= strlen(szSrc);
	nFind= strlen(szFind);

	for( i=k=0 ; k<nFind && i<nSrc ; i++, k++ ) {
		if( toupper(szSrc[i]) != toupper(szFind[k]) ) {
			i -= k;
			k = -1;
		}
	}
	
	if( k==nFind ) {
		return(szSrc + i-nFind);
	}
	return(0);
}

/*********************************************
SHA1 Encoding
*********************************************/
void SHA1Encode(const unsigned char *string, char *result)
{
    int i, len;
    unsigned char hex[20];
    
    SHA1Context sha;
    
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) string, strlen(string));
    
    if (SHA1Result(&sha))
    {
        for(i = 0; i < 5 ; i++)
        {
        	sprintf(result+strlen(result), "%08x", sha.Message_Digest[i]);        	
        };
    };
    //result[strlen(result)] = '\0';
}


/*
#define TESTA   "ZOOC????"

void SHA1Encode(const unsigned char *string, char *result)
{
	SHA1Context sha;
    int i;
	
	SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) TESTA, strlen(TESTA));

    if (!SHA1Result(&sha))
    {        
    }
    else
    {
        for(i = 0; i < 5 ; i++)
        {
            fprintf(stdout, "%.08x", sha.Message_Digest[i]);
        }
        fprintf(stdout, "\n");
    };
};
*/
/*********************************************
left String space delete Method
*********************************************/
char *TrimLeft(char *string)
{
	while(*string) {
		if(isspace(*string)) {
 			++string;
 		}else {
			break;
		}
	}
	return string;
}


/*********************************************
right String space delete Method
*********************************************/
char *TrimRight(char *string)
{
	int len = (int)strlen(string) -1;
	while(len >= 0) {
		if(isspace(*(string+len))) {
			len--;
		}else {
			break;
		}
	}
	*(string + ++len) = 0;
	return string;
}


/*********************************************
String space delete Method
*********************************************/
char *Trim(char *string)
{
	return(TrimLeft(TrimRight(string)));
}



/*********************************************
void euc_to_utf8( const char* str, char* result)
*********************************************/
void euc_to_utf8( const char* str, char* result)
{

	iconv_t cd;
	char* outbuf,* out;
	
	size_t ileft, oleft;
	
	int err;
	int len;

	len = strlen( str );

	ileft = len;
	oleft = len * 2;
	
	out = outbuf = (char*) malloc( oleft + 1 );

	//cd = iconv_open( "UTF-8", "EUC-KR" );//UTF8?? EUCKR??
	cd = iconv_open( "EUC-KR" ,"UTF-8" );//EUCKR?? UTF-8??

	err = iconv( cd, (char**)&str, &ileft, &outbuf, &oleft );
	iconv_close( cd );
	
	sprintf(result, "%s", out);
	result[strlen(out)] = '\0';
	
	//free(outbuf);
	//free(out);
}



/*********************************************
???? ?????? UDP ????
*********************************************/
int UDPSock_CLIENT()
{
	int	sock;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("sock");
		return -1;
	}

    return sock;
}


/*********************************************
??? & ?????? ?????? UDP ????
*********************************************/
int UDPSock_SERVER(int port)
{
	int	sock;
	struct sockaddr_in svrAddr;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("sock");
		return -1;
	}

	memset(&svrAddr, 0x00, sizeof(svrAddr));
	
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = INADDR_ANY;
	svrAddr.sin_port = htons(port);
	
	if(bind(sock, (struct sockaddr *)&svrAddr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
	}

    return sock;
}

/*********************************************
????? ?????? ????????? TCP ????
*********************************************/
int TCPSock_CLIENT(char* ip, int port)
{
	int sock;
    struct	sockaddr_in	svrAddr;
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("sock");
		return -1;
	}
	
	memset(&svrAddr, 0x00, sizeof(svrAddr));
   
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(port);
    svrAddr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sock, (struct sockaddr *)&svrAddr, sizeof(struct sockaddr)) == -1) {
		close(sock);
		return -1;
    }

    return sock;
}


/*********************************************
??? & ?????? ?????? ?????? TCP ????
*********************************************/
int TCPSock_SERVER(int port)
{
    int	sock;
    struct	sockaddr_in	svrAddr;

  	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("sock");
		return -1;
	}
	
	memset(&svrAddr, 0x00, sizeof(svrAddr));
	
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(port);
    svrAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock, (struct sockaddr *)&svrAddr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
    }

    if(listen(sock, 5) == -1) {
		perror("listen");
		exit(1);
    }

    return sock;
}


