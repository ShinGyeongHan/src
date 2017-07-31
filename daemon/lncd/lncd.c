/* ====================================================================
 *
 *  Company     : Copyright ⓒ 2015. All Rights Reserved.
 *  Author      : zakorea@gmail.com
 *	
 ====================================================================*/
 

#include "common.h"

extern SysInfo sysInfo;


stProcessInfo ProcessInfo[] =
{
	{1, 0, MAIN_PROC_FILE_NAME, MAIN_PROC_FILE_NAME_PID},
	{1, 0, LOG_PROC_FILE_NAME, LOG_PROC_FILE_NAME_PID},
	//{1, 0, RTS_PROC_FILE_NAME, RTS_PROC_FILE_NAME_PID},
	{-1, 0, "", ""}
};


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
    	
    	LogPrint(SHOW, __FILE__, "%s[%ld]{%d} %s...Configuration Error", __FILE__, (long)getpid(), __LINE__, __func__ );
    	
    	exit(EXIT_FAILURE);
    };
}


/* ================================================================================================
 * void Get_pid_of_pname()
 *  - pname으로 pid 작성
 * ============================================================================================= */
void Get_pid_of_pname()
{   
	int i, j, pid, procnt;
	char temp[1024];
	char cmd[1024];
	char result[256];
	
	i = 0;
	while(ProcessInfo[i].active > -1)
    {	
    	procnt = 0;
    	sprintf(cmd, CHECKPROCESS, ProcessInfo[i].name);    	
		memset(result, 0x00, sizeof(result));
    	SystemCommandExcute(cmd, result);
    	procnt = atoi(result);
    	
    	if(procnt > 10) {
    		sysInfo.recvTerm = 1;
    		exit(0);
    	}
    	
    	pid = 0;
    	sprintf(cmd, PIDfromPNAME, ProcessInfo[i].name);    	
		memset(result, 0x00, sizeof(result));
    	SystemCommandExcute(cmd, result);
    	pid = atoi(result);
    	
    	if(pid > 0) bPut_pid_to_file(ProcessInfo[i].pid, pid);
    	else remove(ProcessInfo[i].pid);
    	
    	i++;
    	
    };
};


/* ================================================================================================
 * void EngineProcess_Check()
 *  - 프로세스 가동 상태 확인
 * ============================================================================================= */
void EngineProcess_Check()
{
	int i, pid;
	
	//////////////////////////////////////
    /*	각 프로세스 상태 확인하는 구간	*/
    i = 0;
    while(ProcessInfo[i].active > -1)
    {	
    	ProcessInfo[i].status = 0;    	
    	
		if (bIs_there_file(ProcessInfo[i].pid) > 0)
    	{
	    	if ( (pid = bGet_pid_from_file(ProcessInfo[i].pid)) > 0 )
		    {
		        if ( (bIs_there_process_pid(pid)) == 0 )
		        {
		            ProcessInfo[i].status = 1;
		        };
		    };
		};
		
		i++;
	};
}



/* ================================================================================================
 * int doStopCode()
 *  - 데몬 프로세스의 하위 프로세스들에 대한 종료
 * ============================================================================================= */
int doStopCode()
{
    int i, pid;
        
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...start", __FILE__, (long)getpid(), __LINE__, __func__);
	
	//////////////////////////////////////////////
    /*	각 실행 프로세스(pid파일이 존재) 종료	*/
    i = 0;
    while(ProcessInfo[i].active > -1)
    {
	    if (bIs_there_file(ProcessInfo[i].pid) > 0)
	    {
	        if ( (pid = bGet_pid_from_file(ProcessInfo[i].pid)) > 0 )
	        {
	            if ( bIs_there_process_pid(pid) == 0 )
	            {
	            	// 15 SIGTERM(TERM) : Terminate의 약자로 가능한 정상 종료
	                //kill(pid, SIGTERM);
	                
	                // 9 SIGKILL(KILL) : 무조건 종료, 즉 강제 종료
	                kill(pid, SIGKILL);
	                
	                
	                remove(ProcessInfo[i].pid);
	                	                
				};
				
				usleep(DELAY_usleep);
				
	        };
	    }
	    
		i++;
	};
	    
    return 0;
}



/* ================================================================================================
 * int doStartupCode()
 *  - 데몬 프로세스의 하위 프로세스들에 대한 상태 모니터링
 * ============================================================================================= */
int doStartupCode()
{
    int i, pid;
    char _processPath[100];
    

    LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...start", __FILE__, (long)getpid(), __LINE__, __func__ );

     
    //////////////////////////
    /*	Loop 프로세스 시작	*/
    while (ON)
    {
    	        
        
        //////////////////////////
        //pname PID 추출
		Get_pid_of_pname();
		
		
		// 데몬관리(adnengine) 프로세스로부터 종료 시그널 수신시 프로그램 종료
        if ( sysInfo.recvTerm == ON )
        {
            LogPrint(SHOW, __FILE__, "%s[%ld]{%d} %s...receive signal=SigTERM", __FILE__, (long)getpid(), __LINE__, __func__);
            break;
        };
		
		
		//////////////////////////
		/*	프로세스 상태 확인	*/
    	EngineProcess_Check();
    	
    	
    	//////////////////////////////////////
	    /*	각 프로세스 가동하기 위한 구간	*/
	    i = 1;
	    while(ProcessInfo[i].active > 0)
	    {
			if (ProcessInfo[i].active == 1 && ProcessInfo[i].status == 0)
			{
				sprintf(_processPath, "%s/%s", DAEMON_BIN, ProcessInfo[i].name);
				
			    pid = fork();
			    if (pid == 0)
			    {
			        execlp(_processPath, ProcessInfo[i].name, NULL);
			    }
			    else if (pid > 0)
	        	{
	            	ProcessInfo[i].status = 1;
	            	LogPrint(SHOW, __FILE__, "%s[%ld]{%d} %s...process start(%s[%d])", __FILE__, (long)getpid(), __LINE__, __func__, ProcessInfo[i].name, pid);
	        	}
	        	else if(pid < 0)
		        {
		            LogPrint(SHOW, __FILE__, "%s[%ld]{%d} %s...fork error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, ProcessInfo[i].name);
		            ProcessInfo[i].status = 0;
		        };
			};
			i++;
		};
    	
    	// waiting
        sleep(DELAY_sleep);
        
    };
    
    
    //////////////////////
	/*	프로세스 종료	*/
	doStopCode();
	
		
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...end", __FILE__, (long)getpid(), __LINE__, __func__ );

    return 0;

}



/* ================================================================================================
 * int doDaemonStatus(int print)
 *  - 데몬 프로세스의 하위 프로세스들에 대한 상태 출력
 * ============================================================================================= */
int doDaemonStatus()
{
    int i, pid, ret;
	char cmd[256];
	
	/**
    * 각 프로세스 상태를 확인 하기 위한 처리 위한 구간
 	**/
    i = 0;
    while(ProcessInfo[i].active > -1) {
		
		if (bIs_there_file(ProcessInfo[i].pid) > 0)
	    {
			if(( pid = bGet_pid_from_file(ProcessInfo[i].pid) ) > 0 )
		    {
		        if(( ret = bIs_there_process_pid(pid)) == 0 )
		        {
		        	sprintf(cmd, "%s[%d] is running, [%d].\n", ProcessInfo[i].name, pid, ret);
		        	LogPrint(SHOW, __FILE__, cmd);
		        }
		        else
		        {
		        	sprintf(cmd, "%s[%d] is not running, [%d].\n", ProcessInfo[i].name, pid, ret);
		        	LogPrint(SHOW, __FILE__, cmd);
		        };
		    };
		};
		
		i++;
	};

    return 0;

}


/* ================================================================================================
 * void doServerStatus()
 *  - 서버 상태 출력
 * ============================================================================================= */
void doServerStatus()
{   
	float disk1, disk2;
	float cpuload;
	float memory;
	
	char cmd[1024];
	char result[256];
			
	// diskusage command 이용해서 DISK 사용량값    
    sprintf(cmd, CHECKDISK, "/");
	memset(result, 0x00, sizeof(result));
    SystemCommandExcute(cmd, result);
    disk1 = atoi(result);
    
    sprintf(cmd, CHECKDISK, "/data");
	memset(result, 0x00, sizeof(result));
    SystemCommandExcute(cmd, result);
    disk2 = atoi(result);    
    
    // cpuload command 이용해서 CPU 로드율값
    sprintf(cmd, CHECKCPU);
    memset(result, 0x00, sizeof(result));
    SystemCommandExcute(cmd, result);
    cpuload = atoi(result);
    
    // memload command 이용해서 MEMORY 로드율값
    sprintf(cmd, CHECKMEM);
    memset(result, 0x00, sizeof(result));
    SystemCommandExcute(cmd, result);    
    memory = atoi(result);
    
    //printf(" DiskUsage [/]: [%0.0f%]\n DiskUsage [/data]: [%0.0f%]\n cpuLoad : [%0.2f]\n memoryLoad : [%0.2f]\n", disk1, disk2, cpuload, memory);
    sprintf(cmd, " DiskUsage [/]: [%0.0f%]\n DiskUsage [/data]: [%0.0f%]\n cpuLoad : [%0.2f]\n memoryLoad : [%0.2f]\n", disk1, disk2, cpuload, memory);
    LogPrint(SHOW, __FILE__, cmd);
    
}


/* ================================================================================================
 *  usage
 * ============================================================================================= */
void usage()
{
    fprintf( stderr, "Copyright ⓒ 2016 Like&Click Inc. All Rights Reserved.\n");
    fprintf( stderr, "Usage : lncd {start|stop|show|stat}\n");
    fprintf( stderr, "\tlncd {start} All Daemon start\n");
    fprintf( stderr, "\tlncd {stop} All Daemon stop\n");
    fprintf( stderr, "\tlncd {show} Show Daemon Status\n");
    fprintf( stderr, "\tlncd {stat} System Status\n");
    exit(EXIT_FAILURE);
}


/* ================================================================================================
 * int main(int argc, char *argv[])
 * ============================================================================================= */
int main(int argc, char *argv[])
{
    int pid;
    
    
    if ( argc < 2 ) usage();
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SigHandler);
    
        
    MainInitConf();
    
		
	//////////////////////
	/*	프로세스 가동	*/
    if ( strcmp(argv[1], "start") == 0)
	{
		// pid 파일 조회
		if (bIs_there_file(MAIN_PROC_FILE_NAME_PID) > 0)
	    {
	        if ( (pid = bGet_pid_from_file(MAIN_PROC_FILE_NAME_PID)) > 0 )
	        {
	            if ( bIs_there_process_pid(pid) == 0 )
	            {
	            	LogPrint(SHOW, __FILE__, "(pid %d) already running", pid);	            	
	                exit(EXIT_FAILURE);
	            };
	        };
	    }
	    
        //////////////////////////////
        /*	cm  프로세스 시작	*/
        pid = fork();
        if (pid == 0)
        {
        	LogPrint(SHOW, __FILE__, "Process is Starting...");
        	
        	bPut_pid_to_file(MAIN_PROC_FILE_NAME_PID, getpid());
        	
            //////////////////////
            /*	프로세스 시작	*/
            doStartupCode();
            
		}else{
            exit(EXIT_FAILURE);
        };
    }
    
    //////////////////////
	/*	프로세스 중지	*/
    else if ( strcmp(argv[1], "stop") == 0 )
    {		
	    //////////////////////
        /*	프로세스 종료	*/
		doStopCode();		
    }
    
    //////////////////////////
	/*	프로세스 상태 확인	*/
    else if ( strcmp(argv[1], "show") == 0 )
    {
        /* print stdout */
        doDaemonStatus();
    }
    
    //////////////////////////////////
	/*	프로세스 상태 DB업데이트	*/
    else if ( strcmp(argv[1], "stat") == 0 )
    {
        doServerStatus();
    }
    
    else
    {
        usage();
    };
    

    return 0;
}



