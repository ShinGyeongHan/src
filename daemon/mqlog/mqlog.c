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


void* RecvLOGWrite()
{		
    int cntLen = 0;
	int Result = 0;
	int Sock_LOG;
	
	time_t	pt;
	struct	tm	lt;	
		
	char RecvPacket[LOG_PACKET_LIMIT];

	struct sockaddr_in cntAddr;
	
	    
	
	/**************************************
	Client UDP Socket OPEN
	**************************************/
	Sock_LOG = UDPSock_SERVER(39811);                                                         
	
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
		
}


void* LogINIT()
{
	
	time_t	pt;
	struct	tm	lt;	
	char LOGFILE[100];
	    
        
    time(&pt);
	localtime_r(&pt, &lt);
    sprintf(LOGFILE, "%s/%04d%02d%02d.log", DAEMON_MQLIKE, lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);
	fp = fopen(LOGFILE,"a");

	/**********************************************/
	// Log Receive File Write Process
	/**********************************************/
	while(ON)
	{	
		
		time(&pt);
		localtime_r(&pt, &lt);
		sprintf(LOGFILE, "%s/%04d%02d%02d.log", DAEMON_MQLIKE, lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);

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
    

    
    if((pthread_result = pthread_create( &thread1, NULL, LogINIT, NULL))) {
    	
    };
    
    if((pthread_result = pthread_create( &thread2, NULL, RecvLOGWrite, NULL))) {
    	
    };


	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);
	

    pthread_detach(thread1);
    pthread_detach(thread2);


    
    return 0;
    
};

