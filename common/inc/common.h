/* ====================================================================
 *
 *  Company     : Copyright ⓒ 2016. All Rights Reserved.
 *  Author      : zakorea@gmail.com
 *	
 ====================================================================*/

#ifndef COMMON_SHARE_HEADER_DEF
#define COMMON_SHARE_HEADER_DEF
                  
                          
#include <stdio.h>
#include <stdbool.h>
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
#include <limits.h>          
#include <libgen.h>


/*********************************************
	EXTENSION TAG
*********************************************/
#define	MAIN_CMD		0x01
	
#define	CALL_LOG		0x01
#define	VIEW_LOG		0x02
#define	CLICK_LOG		0x03
#define	POST_LOG		0x04
      
#define TOKEN			0x14
                         
#define DELAY_usleep	1000	//1ms, (1/1000000)
#define DELAY_sleep		1		// 1

#define DELAY_TIMEOUT	10
#define usleep_DELAY	1000
#define sleep_DELAY		5

#define DEFAULT			1
#define SUCCESS			1
#define FAIL			-1
#define DEBUG			0
#define SHOW			2
#define SQL_DEBUG		3

#define ON				1
#define OFF				0
#define WAIT			2

typedef unsigned int uint;
typedef unsigned short ushort; 
typedef unsigned char uchar;
typedef unsigned long long ullong;


#define 	UTP_RECV_LIMIT		65536
#define 	UTP_SEND_LIMIT		65536
#define		LOG_PACKET_LIMIT	1024*10

#define WATCHCODE			"NTS9FOW"


/**********************************************/
//1000003,2000003,3000017,4000037,5000011,6000011,7000003,8000009,9000011,
//10000019,20000003,30000001,40000003,50000017,60000011,70000027,80000023,90000049, 100000037
//2147483647,
/**********************************************/
#define MEMORY_LIMIT_CNT	90000049



//type1
#define DEFINE_LOG_CLICK		1		//type=C
#define DEFINE_LOG_POSTBACK		2		//type=P
#define DEFINE_LOG_LIKE			3		//type=K
#define DEFINE_LOG_JOIN			4		//type=J
#define DEFINE_LOG_LOGIN		5		//type=L
#define DEFINE_LOG_VALID		6		//type=V 리포트대상 로그값
#define DEFINE_LOG_NCLICK		7		//type=T 비과금 클릭



//type2
#define DEFINE_TYPE_INSTALL		1		//type2=INSTALL
#define DEFINE_TYPE_POSTBACK	2
#define DEFINE_TYPE_CANCEL		3		//type2=cancel
#define DEFINE_TYPE_LOGIN		4		//type2=login
#define DEFINE_TYPE_ABUSING		5		//type2=abusing
#define DEFINE_TYPE_CPI			6
#define DEFINE_TYPE_CPL			7
#define DEFINE_TYPE_CPC			8
#define DEFINE_TYPE_CPS			9
#define DEFINE_TYPE_SHARE		10
#define DEFINE_TYPE_CPA			11

//type3
#define DEFINE_TYPE3_SHARE_AUTO			1
#define DEFINE_TYPE3_SHARE_SELF			2
#define DEFINE_TYPE3_CANCEL_FACEBOOK	3
#define DEFINE_TYPE3_LOGIN_WEB			4
#define DEFINE_TYPE3_LOGIN_AOS			5
#define DEFINE_TYPE3_LOGIN_IOS			6
#define DEFINE_TYPE3_NCLICK				7

//ads_type
#define DEFINE_ADSTYPE_CPI				1
#define DEFINE_ADSTYPE_CPL				2
#define DEFINE_ADSTYPE_CPC				3
#define DEFINE_ADSTYPE_CPS				4
#define DEFINE_ADSTYPE_CPA				5



#define	DBIP	"likenclick.cw8gzvddyqwz.ap-northeast-2.rds.amazonaws.com"
#define	DBID	"likenclick"
#define	DBPW	"fkdlzmdoszmfflr2017!@!@"
#define	DBNAME	"z_likenclick"
#define	DBPORT	3306

/*********************************************
	설치 경로 설정
*********************************************/

#define DAEMON_HOME  		"/data/ADSERVER"
#define DAEMON_BIN   		DAEMON_HOME "/bin"
#define DAEMON_LOG   		DAEMON_HOME "/log"
#define DAEMON_DATA   		DAEMON_HOME "/data"
#define DAEMON_MQLIKE  		DAEMON_HOME "/mqlike"
#define DAEMON_PID   		DAEMON_HOME "/pid/"
#define	CONF_FILE			DAEMON_HOME "/etc/engine.conf"

/*********************************************
	프로세스 명칭
*********************************************/
#define	MAIN_PROC_FILE_NAME		"lncd"
#define	LOG_PROC_FILE_NAME		"lncd_log"
#define	RTS_PROC_FILE_NAME		"lncd_rts"
#define	NLOG_PROC_FILE_NAME		"nlogday"

#define	MAIN_PROC_FILE_NAME_PID		DAEMON_PID MAIN_PROC_FILE_NAME
#define	LOG_PROC_FILE_NAME_PID		DAEMON_PID LOG_PROC_FILE_NAME
#define	RTS_PROC_FILE_NAME_PID		DAEMON_PID RTS_PROC_FILE_NAME
#define NLOG_PROC_FILE_NAME_PID		DAEMON_PID NLOG_PROC_FILE_NAME


/*********************************************
	프로세스 로그 파일
*********************************************/
#define DAEMON_ERROR_FILE	"error.log"
#define DAEMON_SQL_FILE		"sql.log"

#define CHECKDISK		"df %s | grep -v 'File' | tr -d '\n' | awk '{print$5}' | sed 's/%//g'"
#define CHECKCPU		"vmstat -n 1 2|tail -n 1 | awk '{print 100-$15}'"
#define CHECKMEM		"free | grep Mem | awk '{print $3/$2*100}'"
#define CHECKPROCESS	"ps -ef | grep %s | grep -v grep | wc -l"
#define PIDfromPNAME 	"ps -ef | grep %s | grep -v grep | awk '{print $2}'"


#define CodePlusPacket (SendPacket+strlen(SendPacket))



/*********************************************
프로세스명 및 PID 구조체 저장
*********************************************/
typedef struct {
	int		active;
	int		status;
    char	name[50];
    char	pid[50];
}stProcessInfo;

typedef struct {
			
	int DEBUGMODE;

	int LOG_RECIVE_PORT;
	
	char RTS_SERVER_IP[16];
	
	int RTS_SERVER_PORT;
	int RTS_REPORT1_PORT;
	int RTS_REPORT2_PORT;
	int RTS_REPORT3_PORT;
	int RTS_REPORT4_PORT;
	int RTS_REPORT5_PORT;

    int  recvSig;
    int  recvTerm;
    
    
} SysInfo;




extern int Configuration();
extern void checkTime(char *result);
extern void LogPrint(int logKind, char *logDest, char *fmt, ...);
extern int SystemCommandExcute(char *cmd, char *result);
extern int bPut_pid_to_file(char *fname_pid, int pid);
extern int bIs_there_file(char *filename);
extern int bGet_pid_from_file(char *fname_pid);
extern int bIs_there_process_pid(int pid);
extern int UDPSock_CLIENT();
extern int UDPSock_SERVER(int port);

#endif


