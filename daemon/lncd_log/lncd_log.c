/* ====================================================================
 *
 *  Company     : Copyright ⓒ 2016. All Rights Reserved.
 *  Author      : zakorea@gmail.com
 *	
 ====================================================================*/


#include <pthread.h>


/*********************************************/
// definition
/*********************************************/
#include "common.h"

extern SysInfo sysInfo;

FILE* fp;

bool FLAG_INIT = true;



/* ================================================================================================
 * void SigHandler(int sig)
 * - 시그널 관리 함수
 * ============================================================================================= */
static void SigHandler(int sig)
{
    switch (sig)
    {
	    case SIGTERM :
	    case SIGINT	 :
	        sysInfo.recvTerm = 1;
	        break;
	    case SIGALRM :
	        sysInfo.recvSig = 1;
	        break;
    };
}


/* ================================================================================================
 * void MainInitConf()
 * - 환경 초기화 함수
 * ============================================================================================= */
static void MainInitConf()
{
	if(Configuration() == FAIL)
    {

    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Configuration Error", __FILE__, (long)getpid(), __LINE__, __func__ );

    	exit(EXIT_FAILURE);
    };
}


void* RecvLOGWrite()
{		
    int cntLen = 0;
	int Result = 0;
	int Sock_LOG, Sock_RTS, Sock_FREQ;
	
	time_t	pt;
	struct	tm	lt;	
		
	char RecvPacket[LOG_PACKET_LIMIT];

	struct sockaddr_in cntAddr;
	struct sockaddr_in SvrAddr;
	
    
    LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);
    
	
	/**************************************
	Client UDP Socket OPEN
	**************************************/
	Sock_LOG = UDPSock_SERVER(sysInfo.LOG_RECIVE_PORT);                                                         

	Sock_RTS = UDPSock_CLIENT();	
	
	memset(&SvrAddr,0,sizeof(SvrAddr));
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(sysInfo.RTS_SERVER_PORT);
	SvrAddr.sin_addr.s_addr = inet_addr(sysInfo.RTS_SERVER_IP);
	


	/**********************************************/
	// Log Receive File Write Process
	/**********************************************/
	while(ON)
	{
		// init
		RecvPacket[0] = '\0';				
		
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(Sock_LOG, RecvPacket, sizeof RecvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		//if(Result < 0) continue;			
		RecvPacket[Result] = 0;
		
		// RTS SEND
		if(Sock_RTS > 0) sendto(Sock_RTS, RecvPacket, strlen(RecvPacket), 0, (struct sockaddr *)&SvrAddr, sizeof(struct sockaddr));

		// 파일 FP 포인터가 만들어 지는 경우, 잠시 대기를 위해
		if(!FLAG_INIT)
		{
			while(ON) {
				if(FLAG_INIT) break;
				usleep(1);
			};
		};	
		
		if(fp) {
			fprintf(fp, "%s\n", RecvPacket);
			fflush(fp);
		};
		
		
	};

	/**************************************
	SOCKET CLOSE()
	**************************************/
	close(Sock_LOG);
	close(Sock_RTS);

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);
	
}


void* LogINIT()
{
	
	time_t	pt;
	struct	tm	lt;	
	char LOGFILE[100];
	    
    LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);
    
    
    time(&pt);
	localtime_r(&pt, &lt);
    sprintf(LOGFILE, "%s/%04d%02d%02d.log", DAEMON_DATA, lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);
	fp = fopen(LOGFILE,"a");

	/**********************************************/
	// Log Receive File Write Process
	/**********************************************/
	while(ON)
	{	
		/*
		if ( sysInfo.recvTerm == ON )
        {
            LogPrint(SHOW, __FILE__,"%s[%ld]{%d} %s...receive signal=SigTERM", __FILE__, (long)getpid(), __LINE__, __func__);
            break;
        };
        */
		
		time(&pt);
		localtime_r(&pt, &lt);
		sprintf(LOGFILE, "%s/%04d%02d%02d.log", DAEMON_DATA, lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);

		if( FLAG_INIT &&  lt.tm_hour == 0 && lt.tm_min == 0 && lt.tm_sec == 0 )
		{
			FLAG_INIT = false;

			if(fp) fclose(fp);
			fp = fopen(LOGFILE,"a");
			FLAG_INIT = true;
			sleep(1);

		}else {

			if ( access(LOGFILE, F_OK) < 0) {

				FLAG_INIT = false;
				if(fp) fclose(fp);
				fp = fopen(LOGFILE,"a");
			};

			FLAG_INIT = true;

		};

		usleep(5);
		
	};

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);

};


/* ================================================================================================
 *  main
 * ============================================================================================= */
int main(int argc, char *argv[])
{    

	int pthread_result;
	pthread_t thread1, thread2;
    
	//시그널 처리
	signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SigHandler);


	MainInitConf();
	
	// PID 작성
    bPut_pid_to_file(LOG_PROC_FILE_NAME_PID, getpid());

    
    LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...ENGINE START", __FILE__, (long)getpid(), __LINE__, __func__);
	
    
    if((pthread_result = pthread_create( &thread1, NULL, LogINIT, NULL))) {
    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...LogINIT creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
    };
    
    if((pthread_result = pthread_create( &thread2, NULL, RecvLOGWrite, NULL))) {
    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...RecvLOGWrite creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
    };


	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);
	

    pthread_detach(thread1);
    pthread_detach(thread2);

	
	// 해당 PID 파일 삭제
	remove(LOG_PROC_FILE_NAME_PID);
	

	
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...ENGINE STOP", __FILE__, (long)getpid(), __LINE__, __func__);

    
    return 0;
    
};

