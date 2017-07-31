                                                          /* ====================================================================
 *
 *  Company     : Copyright 2015 OnuriDMC Inc. All Rights Reserved.
 *  Author      : zakorea@gmail.com                               
 
 ====================================================================*/
 
/*********************************************/
// definition
/*********************************************/

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <pthread.h>

#include "common.h"

#include <mysql.h>                  
#include <search.h>                 


extern SysInfo sysInfo;


//pthread_mutex_t DB_SYNC	= PTHREAD_MUTEX_INITIALIZER;	PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t DB_SYNC_CMP	= PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t DB_SYNC_PUB	= PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t DB_SYNC_ADS	= PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;


/**********************************************/
// CORE_RCV_PROCESS
//1000003,2000003,3000017,4000037,5000011,6000011,7000003,8000009,9000011,
//10000019,20000003,30000001,40000003,50000017,60000011,70000027,80000023,90000049, 100000037
//2147483647,
/**********************************************/
#define MEMORY_LIMIT_CNT	90000049


/*********************************************
//	Hash Memory Setup
/*********************************************/
ENTRY    hash_item;
ENTRY    *hash_result;

/*
type=r
time=1437539012
ip=182.221.88.37
uid=841e751e26080d46e4f4cac8fc3dcb7a
uid_type=cookie
platform=etc
sdk=web
appkey=q02m8ppv
width=320
height=50
rtn=0
pub_idx=19
shop_idx=72
camp_idx=71
camp_type=cpc
ads_idx=1009
price=58
bf=1
tf=AD_NON

url=https://81354.measurementapi.com/serve?action=click&publisher_id=81354&site_id=57394&destination_id=243698&unid={click_id}&ref_id={call_id}&sub1=OD&sub2=coocha&sub3={ref_1}&sub4=gear_0629&sub_campaign=0707_gear_0629&sub_publisher=normalandroid_url=https://81354.measurementapi.com/serve?action=click&publisher_id=81354&site_id=57394&destination_id=243698&unid={click_id}&ref_id={call_id}&sub1=OD&sub2=coocha&sub3={ref_1}&sub4=gear_0629&sub_campaign=0707_gear_0629&sub_publisher=normalios_url=https://81354.measurementapi.com/serve?action=click&publisher_id=81354&site_id=57474&unid={click_id}&ref_id={call_id}&sub1=OD&sub2=coocha&sub3={ref_1}&sub4=gear_0629&sub_campaign=0707_gear_0629&sub_publisher=normal_id=1_55af1ac416a7ec54e58b573d
*/

typedef struct {	
	uchar type;
	uint time;
	uchar userip[16];	
	uchar uid[50];	
	uchar uid_type[10];	
   	uchar platform;
	uchar sdk;
	uchar appkey[10];
	uchar rtn;
	int pub_idx;
	int shop_idx;
	int camp_idx;
	uchar camp_type;
	int ads_idx;
	uint price;
	uchar bf;
	uchar tf;	
}logBUFFER;
logBUFFER	_recvLOG;
logBUFFER	_recvLOGFILE;



typedef struct {
	uint request[60][60];
	uint response[60][60];
	uint noad[60][60];
	uint imp[60][60];
	uint clk[60][60];
	uint clk_cost[60][60];	
	uint postback[60][60];	
}stTOTAL_RTS_TIME;

typedef struct {
	stTOTAL_RTS_TIME hour[24];
}stTOTAL_RTS_TIME_MONTH;

#define	_MAX_MONTH_MEMORY 32
stTOTAL_RTS_TIME_MONTH	_TOTAL_TIME[_MAX_MONTH_MEMORY];


typedef struct {
	uint request;
	uint response;
	uint noad;
	uint imp;
	uint clk;
	uint clk_cost;
	uint postback;
	uint u_request;
	uint u_imp;
	uint u_clk;
	int chk;
}stTOTAL_RTS;

stTOTAL_RTS	_TOTAL;
stTOTAL_RTS	_PLATFORM_ANDROID;
stTOTAL_RTS	_PLATFORM_IOS;
stTOTAL_RTS	_PLATFORM_ETC;
stTOTAL_RTS	_SDK_APP;
stTOTAL_RTS	_SDK_WEB;
stTOTAL_RTS	_SDK_ETC;
stTOTAL_RTS	_CMP_CPM;
stTOTAL_RTS	_CMP_CPC;
stTOTAL_RTS	_CMP_CPI;
stTOTAL_RTS	_TF_TAR;
stTOTAL_RTS	_TF_NON;
stTOTAL_RTS	_TF_TAG;


#define RTS_MEM_LIMIT	10000

stTOTAL_RTS	_CMP[RTS_MEM_LIMIT];
stTOTAL_RTS	_PUB[RTS_MEM_LIMIT];
stTOTAL_RTS	_ADS[RTS_MEM_LIMIT];

typedef struct {
	int idx;
	char name[100];
	bool status;
}stDBINFO;

stDBINFO* _CAMP_INFO = NULL;
int _CAMP_INFOcnt = 0;
stDBINFO* _PUB_INFO = NULL;
int _PUB_INFOcnt = 0;

typedef struct {
	int idx;
	uint camp_idx;
	int type;
	char name[200];
	char img[255];
	int price;
	//bool status;
}stDBINFO2;
stDBINFO2* _ADS_INFO = NULL;
int _ADS_INFOcnt = 0;


struct thread_arg {
	pthread_t pt_id;
	int idx;
}thr_arg[_MAX_MONTH_MEMORY];


/*
typedef int (*comp)(const void* A, const void* B);
int ads_compare(const stTOTAL_RTS* A, const stTOTAL_RTS* B)
{
    return A->imp < B->imp;
}
*/

/*
//1000003,2000003,3000017,4000037,5000011,6000011,7000003,8000009,9000011,
//10000019,20000003,30000001,40000003,50000017,60000011,70000027,80000023,90000049
#define	FREQ_MEM_LIMIT		50000017
typedef struct {
	int imp;
}stFREQ;
stFREQ _Freq[FREQ_MEM_LIMIT];
static int _Freqcnt = 0;
static uint _FreqRTS[51] = {0,};
*/

//bool INIT_LOG = true;
uint CHECK_TIME = 0;

bool FLAG_INIT = true;            


/* ================================================================================================
 * void SigHandler(uint sig)
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
 * ============================================================================================= */
static void MainInitConf()
{
	if(Configuration() == FAIL)
    {

    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Configuration Error", __FILE__, (long)getpid(), __LINE__, __func__ );

    	exit(EXIT_FAILURE);
    };
}





/*********************************************/
// setFreqUpdate
/*********************************************/
#if 0
void setFreqUpdate(int rimp, int imp)
{

	if(imp == 1) {
		_FreqRTS[1]++;
	}else if(imp > 49) {
		_FreqRTS[50]++;
		if(rimp > 0) _FreqRTS[rimp]--;
	}else {
		_FreqRTS[imp]++;
		if(rimp > 0) _FreqRTS[rimp]--;
	}
};
#endif



/*********************************************/
// Log Line => Parsing
/*********************************************/
int LineParser(char* p)
{
	int i, n;
	char* m;
	char Line[1024], name[20], val[1000];
	char tmp;

	memset(&_recvLOG, 0, sizeof(_recvLOG));

   	_recvLOG.sdk = DEFINE_SDK_ETC;
   	_recvLOG.platform = DEFINE_PLATFORM_ETC;

	replace(p, " ", "");

	m = p;

   	n = 0;
	while(*p != 0)
	{
		if(*p == TOKEN)
		{
			*p = ' ';
			n++;
   		};
   		p++;
   	};
	n++;

	for(i = 0 ; i < n  ; i++)
	{

		memset(name, 0, sizeof(name));
   		memset(val, 0, sizeof(val));

		sscanf(m, "%s", Line);

		p = strchr(Line, '=');
   		if(p == NULL) break;

		*p = ' ';
   		sscanf(Line, "%s%s", name, val);


   		if(strcmp(name, "type") == 0) {

   			strcpy(&tmp, val);

           if( tmp == 'r') 							_recvLOG.type = DEFINE_LOG_REQUEST;
           else if( tmp == 'v') 					_recvLOG.type = DEFINE_LOG_VIEW;
           else if( tmp == 'c') 					_recvLOG.type = DEFINE_LOG_CLICK;
           else if( tmp == 'p') 					_recvLOG.type = DEFINE_LOG_POSTBACK;
        }

        else if(strcmp(name, "time") == 0)          _recvLOG.time = atoi(val);
        else if(strcmp(name, "ip") == 0)            strcpy(_recvLOG.userip,val);
        else if(strcmp(name, "uid") == 0)           strcpy(_recvLOG.uid,val);
        else if(strcmp(name, "uid_type") == 0)      strcpy(_recvLOG.uid_type,val);

        else if(strcmp(name, "platform") == 0) 		{
            if( strcmp(val, "android") == 0 )       _recvLOG.platform = DEFINE_PLAYFORM_AD;
            else if( strcmp(val, "ios") == 0 )      _recvLOG.platform = DEFINE_PLATFORM_IOS;
            else 									_recvLOG.platform = DEFINE_PLATFORM_ETC;
        }

        else if(strcmp(name, "sdk") == 0) 			{
            if( strcmp(val, "app") == 0 )           _recvLOG.sdk = DEFINE_SDK_APP;
            else if( strcmp(val, "web") == 0 )      _recvLOG.sdk = DEFINE_SDK_WEB;
            else 									_recvLOG.sdk = DEFINE_SDK_ETC;
    	}

        else if(strcmp(name, "rtn") == 0) 			{
            if( atoi(val) == 0 ) 					_recvLOG.rtn = DEFINE_RTN_ISAD;
            else 									_recvLOG.rtn = DEFINE_RTN_NOAD;
        }

        else if(strncmp(name, "pub_idx", 7) == 0)       _recvLOG.pub_idx = atoi(val);
        else if(strncmp(name, "shop_idx", 8) == 0)      _recvLOG.shop_idx = atoi(val);
        else if(strncmp(name, "camp_idx", 8) == 0)      _recvLOG.camp_idx = atoi(val);

        else if(strncmp(name, "camp_type", 9) == 0) 	{
            if( strcmp(val, "cpc") == 0 )           _recvLOG.camp_type = DEFINE_CAMP_TYPE_CPC;
            else if( strcmp(val, "cpm") == 0 )      _recvLOG.camp_type = DEFINE_CAMP_TYPE_CPM;
            else if( strcmp(val, "cpi") == 0 )      _recvLOG.camp_type = DEFINE_CAMP_TYPE_CPI;
        }

        else if(strncmp(name, "ads_idx", 7) == 0)       _recvLOG.ads_idx = atoi(val);
        else if(strcmp(name, "price") == 0)         _recvLOG.price = atoi(val);
        else if(strcmp(name, "bf") == 0)            _recvLOG.bf = atoi(val);

        else if(strcmp(name, "tf") == 0) 			{
            if( strcmp(val, "AD_RET") == 0 )        _recvLOG.tf = DEFINE_TF_ADRET;
            else if( strcmp(val, "AD_NON") == 0 )   _recvLOG.tf = DEFINE_TF_ADNON;
            else if( strcmp(val, "AD_TAG") == 0 )   _recvLOG.tf = DEFINE_TF_ADTAG;
        }


   		m = strchr(m, ' ');
		m++;
	};

	return SUCCESS;

};

void set_total_memory(uchar type, uchar rtn, uint price, int d, int h, int m, int s)
{

	switch(type)
	{
		case DEFINE_LOG_REQUEST :
			
			_TOTAL.request++;
			_TOTAL_TIME[d].hour[h].request[m][s]++;

			if(rtn == DEFINE_RTN_ISAD) {
				_TOTAL.response++;
				_TOTAL_TIME[d].hour[h].response[m][s]++;
			}
			else
			{
				_TOTAL.noad++;
				_TOTAL_TIME[d].hour[h].noad[m][s]++;
			}

			break;

		case DEFINE_LOG_VIEW :
	
			_TOTAL.imp++;
			_TOTAL_TIME[d].hour[h].imp[m][s]++;

			break;

		case DEFINE_LOG_CLICK :

			_TOTAL.clk++;
			_TOTAL_TIME[d].hour[h].clk[m][s]++;
			
			_TOTAL.clk_cost += price;
			_TOTAL_TIME[d].hour[h].clk_cost[m][s] += price;

			break;

		case DEFINE_LOG_POSTBACK :

			_TOTAL.postback++;
			_TOTAL_TIME[d].hour[h].postback[m][s]++;
			break;
	};
};



void set_platform_memory(uchar type, uchar flag, uchar rtn, uint price)
{

    switch(type)
	{
		case DEFINE_LOG_REQUEST :

			if(flag == DEFINE_PLAYFORM_AD) {

				_PLATFORM_ANDROID.request++;

				if(rtn == DEFINE_RTN_ISAD) _PLATFORM_ANDROID.response++;
				else _PLATFORM_ANDROID.noad++;

			}else if(flag == DEFINE_PLATFORM_IOS) {

				_PLATFORM_IOS.request++;

				if(rtn == DEFINE_RTN_ISAD) _PLATFORM_IOS.response++;
				else _PLATFORM_IOS.noad++;

			//}else if(flag == DEFINE_PLATFORM_ETC) {
			}else {
				_PLATFORM_ETC.request++;

				if(rtn == DEFINE_RTN_ISAD) _PLATFORM_ETC.response++;
				else _PLATFORM_ETC.noad++;
			}

			break;

		case DEFINE_LOG_VIEW :

			if(flag == DEFINE_PLAYFORM_AD) {

				_PLATFORM_ANDROID.imp++;

			}else if(flag == DEFINE_PLATFORM_IOS) {

				_PLATFORM_IOS.imp++;

			//}else if(flag == DEFINE_PLATFORM_ETC) {
			}else {
				_PLATFORM_ETC.imp++;

			}

			break;

		case DEFINE_LOG_CLICK :

			if(flag == DEFINE_PLAYFORM_AD) {

				_PLATFORM_ANDROID.clk++;
				_PLATFORM_ANDROID.clk_cost += price;

			}else if(flag == DEFINE_PLATFORM_IOS) {

				_PLATFORM_IOS.clk++;
				_PLATFORM_IOS.clk_cost += price;

			//}else if(flag == DEFINE_PLATFORM_ETC) {
			}else {

				_PLATFORM_ETC.clk++;
				_PLATFORM_ETC.clk_cost += price;

			}

			break;

		case DEFINE_LOG_POSTBACK :

			if(flag == DEFINE_PLAYFORM_AD) {

				_PLATFORM_ANDROID.postback++;

			}else if(flag == DEFINE_PLATFORM_IOS) {

				_PLATFORM_IOS.postback++;

			//}else if(flag == DEFINE_PLATFORM_ETC) {
			}else {

				_PLATFORM_ETC.postback++;

			}

			break;
	};
}




void set_sdk_memory(uchar type, uchar flag, uchar rtn, uint price)
{

    switch(type)
	{
		case DEFINE_LOG_REQUEST :

			if(flag == DEFINE_SDK_APP) {

				_SDK_APP.request++;

				if(rtn == DEFINE_RTN_ISAD) _SDK_APP.response++;
				else _SDK_APP.noad++;

			}else if(flag == DEFINE_SDK_WEB) {

				_SDK_WEB.request++;

				if(rtn == DEFINE_RTN_ISAD) _SDK_WEB.response++;
				else _SDK_WEB.noad++;

			//}else if(flag == DEFINE_SDK_ETC) {
			}else {

				_SDK_ETC.request++;

				if(rtn == DEFINE_RTN_ISAD) _SDK_ETC.response++;
				else _SDK_ETC.noad++;
			}

			break;

		case DEFINE_LOG_VIEW :

			if(flag == DEFINE_SDK_APP) {

				_SDK_APP.imp++;

			}else if(flag == DEFINE_SDK_WEB) {

				_SDK_WEB.imp++;

			//}else if(flag == DEFINE_SDK_ETC) {
			}else {
				_SDK_ETC.imp++;

			}

			break;

		case DEFINE_LOG_CLICK :

			if(flag == DEFINE_SDK_APP) {

				_SDK_APP.clk++;
				_SDK_APP.clk_cost += price;

			}else if(flag == DEFINE_SDK_WEB) {

				_SDK_WEB.clk++;
				_SDK_WEB.clk_cost += price;

			//}else if(flag == DEFINE_SDK_ETC) {
			}else {

				_SDK_ETC.clk++;
				_SDK_ETC.clk_cost += price;

			}

			break;

		case DEFINE_LOG_POSTBACK :

			if(flag == DEFINE_SDK_APP) {

				_SDK_APP.postback++;

			}else if(flag == DEFINE_SDK_WEB) {

				_SDK_WEB.postback++;

			//}else if(flag == DEFINE_SDK_ETC) {
			}else {
				_SDK_ETC.postback++;
			}

			break;
	};
}



void set_cmp_memory(uchar type, uchar flag, uchar rtn, uint price)
{

    switch(type)
	{
		case DEFINE_LOG_REQUEST :

			if(flag == DEFINE_CAMP_TYPE_CPC) {

				_CMP_CPC.request++;

				if(rtn == DEFINE_RTN_ISAD) _CMP_CPC.response++;
				else _CMP_CPC.noad++;

			}else if(flag == DEFINE_CAMP_TYPE_CPM) {

				_CMP_CPM.request++;

				if(rtn == DEFINE_RTN_ISAD) _CMP_CPM.response++;
				else _CMP_CPM.noad++;

			}else if(flag == DEFINE_CAMP_TYPE_CPI) {

				_CMP_CPI.request++;

				if(rtn == DEFINE_RTN_ISAD) _CMP_CPI.response++;
				else _CMP_CPI.noad++;
			}

			break;

		case DEFINE_LOG_VIEW :

			if(flag == DEFINE_CAMP_TYPE_CPC) {

				_CMP_CPC.imp++;

			}else if(flag == DEFINE_CAMP_TYPE_CPM) {

				_CMP_CPM.imp++;

			}else if(flag == DEFINE_CAMP_TYPE_CPI) {

				_CMP_CPI.imp++;

			}

			break;

		case DEFINE_LOG_CLICK :

			if(flag == DEFINE_CAMP_TYPE_CPC) {

				_CMP_CPC.clk++;
				_CMP_CPC.clk_cost += price;

			}else if(flag == DEFINE_CAMP_TYPE_CPM) {

				_CMP_CPM.clk++;
				_CMP_CPM.clk_cost += price;

			}else if(flag == DEFINE_CAMP_TYPE_CPI) {

				_CMP_CPI.clk++;
				_CMP_CPI.clk_cost += price;

			}

			break;

		case DEFINE_LOG_POSTBACK :

			if(flag == DEFINE_CAMP_TYPE_CPC) {

				_CMP_CPC.postback++;

			}else if(flag == DEFINE_CAMP_TYPE_CPM) {

				_CMP_CPM.postback++;

			}else if(flag == DEFINE_CAMP_TYPE_CPI) {

				_CMP_CPI.postback++;

			}

			break;
	};
}




void set_tf_memory(uchar type, uchar flag, uchar rtn, uint price)
{

    switch(type)
	{
		case DEFINE_LOG_REQUEST :

			if(flag == DEFINE_TF_ADRET) {
				//_TF_TAR.request++;
				if(rtn == DEFINE_RTN_ISAD) _TF_TAR.response++;
				else _TF_TAR.noad++;
			}else if(flag == DEFINE_TF_ADNON) {
				//_TF_NON.request++;
				if(rtn == DEFINE_RTN_ISAD) _TF_NON.response++;
				else _TF_NON.noad++;					
			}else if(flag == DEFINE_TF_ADTAG) {
				if(rtn == DEFINE_RTN_ISAD) _TF_TAG.response++;
				else _TF_TAG.noad++;
			}
			
			break;

		case DEFINE_LOG_VIEW :

			if(flag == DEFINE_TF_ADRET) {
				_TF_TAR.imp++;
			}else if(flag == DEFINE_TF_ADNON) {
				_TF_NON.imp++;
			}else if(flag == DEFINE_TF_ADTAG) {
				_TF_TAG.imp++;
			}

			break;

		case DEFINE_LOG_CLICK :

			if(flag == DEFINE_TF_ADRET) {
				_TF_TAR.clk++;
				_TF_TAR.clk_cost += price;
			}else if(flag == DEFINE_TF_ADNON) {
				_TF_NON.clk++;
				_TF_NON.clk_cost += price;
			}else if(flag == DEFINE_TF_ADTAG) {
				_TF_TAG.clk++;
				_TF_TAG.clk_cost += price;
			}

			break;

		case DEFINE_LOG_POSTBACK :

			if(flag == DEFINE_TF_ADRET) {
				_TF_TAR.postback++;
			}else if(flag == DEFINE_TF_ADNON) {
				_TF_NON.postback++;
			}else if(flag == DEFINE_TF_ADTAG) {
				_TF_TAG.postback++;
			}

			break;
	};
}



void set_camp_memory(uchar type, int idx, uint price)
{

	_CMP[idx].chk = 1;

    switch(type)
	{
		case DEFINE_LOG_VIEW :
			_CMP[idx].imp++;
			break;

		case DEFINE_LOG_CLICK :
			_CMP[idx].clk++;
			_CMP[idx].clk_cost += price;
			break;

		case DEFINE_LOG_POSTBACK :
			_CMP[idx].postback++;
			break;
	};
}


void set_ads_memory(uchar type, int idx, uint price)
{

	_ADS[idx].chk = 1;

    switch(type)
	{
		case DEFINE_LOG_VIEW :
			_ADS[idx].imp++;
			break;

		case DEFINE_LOG_CLICK :
			_ADS[idx].clk++;
			_ADS[idx].clk_cost += price;
			break;

		case DEFINE_LOG_POSTBACK :
			_ADS[idx].postback++;
			break;
	};

	//qsort((void*)_ADS, RTS_MEM_LIMIT,  sizeof(stTOTAL_RTS), (comp)ads_compare);

}


void set_pub_memory(uchar type, int idx, uchar rtn, uint price)
{

	_PUB[idx].chk = 1;

    switch(type)
	{
		case DEFINE_LOG_REQUEST :

			_PUB[idx].request++;

			if(rtn == DEFINE_RTN_ISAD) _PUB[idx].response++;
			else _PUB[idx].noad++;

			break;

		case DEFINE_LOG_VIEW :
			_PUB[idx].imp++;
			break;

		case DEFINE_LOG_CLICK :
			_PUB[idx].clk++;
			_PUB[idx].clk_cost += price;
			break;

		case DEFINE_LOG_POSTBACK :
			_PUB[idx].postback++;
			break;
	};
}


void* CORE_RCV_PROCESS()
{

	int Result;
	int cntLen;
	int sock;

	char *p;

	char recvPacket[UTP_RECV_LIMIT];

	char tmp[60];
	char uid_sha[51];

	time_t	pt;
	struct	tm	lt;

	int r;
	int _Freqidx;
	
	int d = 0, h = 0, m = 0, s = 0;

	struct sockaddr_in cntAddr;


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);


	sock = UDPSock_SERVER(sysInfo.RTS_SERVER_PORT);


	CHECK_TIME = time(&pt);


	while(ON)
	{
		
		memset(uid_sha, 0x00, sizeof(uid_sha));

		recvPacket[0] = 0;
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(sock, recvPacket, sizeof recvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		if(Result < 0) continue;
		recvPacket[Result] = 0;

		if( recvPacket[0] != 't') continue;

		//printf("%s\n", recvPacket);
        //p = strstr(recvPacket, "url=");
		//if(p) *p = 0;
		
		time(&pt);
		localtime_r(&pt, &lt);
		
		d = lt.tm_mday;
		h = lt.tm_hour;
		m = lt.tm_min;
		s = lt.tm_sec;

	    // Line Parsing
		LineParser(recvPacket);


		if(!FLAG_INIT)
		{
			while(ON) {
				if(FLAG_INIT) break;
				usleep(1);
			};
		};
		
		

		// Set Memory
        set_total_memory(_recvLOG.type, _recvLOG.rtn, _recvLOG.price, d, h, m, s);
        set_platform_memory(_recvLOG.type, _recvLOG.platform, _recvLOG.rtn, _recvLOG.price);
        set_sdk_memory(_recvLOG.type, _recvLOG.sdk, _recvLOG.rtn, _recvLOG.price);
        set_cmp_memory(_recvLOG.type, _recvLOG.camp_type, _recvLOG.rtn, _recvLOG.price);
        set_tf_memory(_recvLOG.type, _recvLOG.tf, _recvLOG.rtn, _recvLOG.price);

		if(_recvLOG.camp_idx > 0) set_camp_memory(_recvLOG.type, _recvLOG.camp_idx, _recvLOG.price);
        if(_recvLOG.pub_idx > 0) set_pub_memory(_recvLOG.type, _recvLOG.pub_idx, _recvLOG.rtn, _recvLOG.price);
        if(_recvLOG.ads_idx > 0) set_ads_memory(_recvLOG.type, _recvLOG.ads_idx, _recvLOG.price);

		if(_recvLOG.type == DEFINE_LOG_REQUEST)
		{
			memset(uid_sha, 0x00, sizeof(uid_sha));
			sprintf(tmp, "r%s", _recvLOG.uid);
			SHA1Encode(tmp, uid_sha);
			hash_item.key = uid_sha;
			if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
			{
				//ÃÂÃÂÃÂÃÂ¬ÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ«ÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ¤!
				hash_item.key	= uid_sha;
				hash_item.data	= "1";
				//hash_item.data	= _recvLOG.uid;
				hsearch(hash_item, ENTER);

				_TOTAL.u_request++;
			};
		}
		else if(_recvLOG.type == DEFINE_LOG_VIEW)
		{

			memset(uid_sha, 0x00, sizeof(uid_sha));
			sprintf(tmp, "v%s", _recvLOG.uid);
			SHA1Encode(tmp, uid_sha);
			hash_item.key = uid_sha;

			if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
			{
				//_Freq[_Freqcnt].imp = 1;
				//setFreqUpdate(0,1);

				hash_item.key	= uid_sha;
				hash_item.data	= "1";
				//hash_item.data	= (void *)_Freqcnt;
				hsearch(hash_item, ENTER);

				_TOTAL.u_imp++;
				//_Freqcnt++;

			}
			/*
			else {
				_Freqidx = -1;
				_Freqidx = (int)(hash_result->data);
				if(_Freqidx > -1) {
					r = _Freq[_Freqidx].imp;
					_Freq[_Freqidx].imp = _Freq[_Freqidx].imp + 1;
					setFreqUpdate(r, _Freq[_Freqidx].imp);
				};
			};
			*/
		}
		else if(_recvLOG.type == DEFINE_LOG_CLICK)
		{
			memset(uid_sha, 0x00, sizeof(uid_sha));
			sprintf(tmp, "c%s", _recvLOG.uid);
			SHA1Encode(tmp, uid_sha);
			hash_item.key = uid_sha;
			if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
			{
				hash_item.key	= uid_sha;
				hash_item.data	= "1";
				//hash_item.data	= _recvLOG.uid;
				hsearch(hash_item, ENTER);
				_TOTAL.u_clk++;
			};
		};
	};

	close(sock);


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);


};



/*********************************************/
// Log Line => Parsing
/*********************************************/
int LogParser(char* p)
{
	int i, n;
	char* m;
	char Line[1024], name[20], val[1000];
	char tmp;

	memset(&_recvLOGFILE, 0, sizeof(_recvLOGFILE));

	_recvLOGFILE.sdk = DEFINE_SDK_ETC;
   	_recvLOGFILE.platform = DEFINE_PLATFORM_ETC;

	replace(p, " ", "");

	m = p;

   	n = 0;
	while(*p != 0)
	{
		if(*p == TOKEN)
		{
			*p = ' ';
			n++;
   		};
   		p++;
   	};
	n++;

	for(i = 0 ; i < n  ; i++)
	{

		memset(name, 0, sizeof(name));
   		memset(val, 0, sizeof(val));

		sscanf(m, "%s", Line);

		p = strchr(Line, '=');
   		if(p == NULL) break;

		*p = ' ';
   		sscanf(Line, "%s%s", name, val);


   		if(strcmp(name, "type") == 0) {

   			strcpy(&tmp, val);

           if( tmp == 'r') 							_recvLOGFILE.type = DEFINE_LOG_REQUEST;
           else if( tmp == 'v') 					_recvLOGFILE.type = DEFINE_LOG_VIEW;
           else if( tmp == 'c') 					_recvLOGFILE.type = DEFINE_LOG_CLICK;
           else if( tmp == 'p') 					_recvLOGFILE.type = DEFINE_LOG_POSTBACK;
        }

        else if(strcmp(name, "time") == 0)          _recvLOGFILE.time = atoi(val);
        else if(strcmp(name, "ip") == 0)            strcpy(_recvLOGFILE.userip,val);
        else if(strcmp(name, "uid") == 0)           strcpy(_recvLOGFILE.uid,val);
        else if(strcmp(name, "uid_type") == 0)      strcpy(_recvLOGFILE.uid_type,val);

        else if(strcmp(name, "platform") == 0) 		{
            if( strcmp(val, "android") == 0 )       _recvLOGFILE.platform = DEFINE_PLAYFORM_AD;
            else if( strcmp(val, "ios") == 0 )      _recvLOGFILE.platform = DEFINE_PLATFORM_IOS;
            else 									_recvLOGFILE.platform = DEFINE_PLATFORM_ETC;
        }

        else if(strcmp(name, "sdk") == 0) 			{
            if( strcmp(val, "app") == 0 )           _recvLOGFILE.sdk = DEFINE_SDK_APP;
            else if( strcmp(val, "web") == 0 )      _recvLOGFILE.sdk = DEFINE_SDK_WEB;
            else 									_recvLOGFILE.sdk = DEFINE_SDK_ETC;

    	}

        else if(strcmp(name, "rtn") == 0) 			{
            if( atoi(val) == 0 ) 					_recvLOGFILE.rtn = DEFINE_RTN_ISAD;
            else 									_recvLOGFILE.rtn = DEFINE_RTN_NOAD;
        }

        else if(strncmp(name, "pub_idx", 7) == 0)       _recvLOGFILE.pub_idx = atoi(val);
        else if(strncmp(name, "shop_idx", 8) == 0)      _recvLOGFILE.shop_idx = atoi(val);
        else if(strncmp(name, "camp_idx", 8) == 0)      _recvLOGFILE.camp_idx = atoi(val);

        else if(strncmp(name, "camp_type", 9) == 0) 	{
            if( strcmp(val, "cpc") == 0 )           _recvLOGFILE.camp_type = DEFINE_CAMP_TYPE_CPC;
            else if( strcmp(val, "cpm") == 0 )      _recvLOGFILE.camp_type = DEFINE_CAMP_TYPE_CPM;
            else if( strcmp(val, "cpi") == 0 )      _recvLOGFILE.camp_type = DEFINE_CAMP_TYPE_CPI;
        }

        else if(strncmp(name, "ads_idx", 7) == 0)       _recvLOGFILE.ads_idx = atoi(val);
        else if(strcmp(name, "price") == 0)         _recvLOGFILE.price = atoi(val);
        else if(strcmp(name, "bf") == 0)            _recvLOGFILE.bf = atoi(val);

        else if(strcmp(name, "tf") == 0) 			{
            if( strcmp(val, "AD_RET") == 0 )        _recvLOGFILE.tf = DEFINE_TF_ADRET;
            else if( strcmp(val, "AD_NON") == 0 )   _recvLOGFILE.tf = DEFINE_TF_ADNON;
            else if( strcmp(val, "AD_TAG") == 0 )   _recvLOGFILE.tf = DEFINE_TF_ADTAG;
        }


   		m = strchr(m, ' ');
		m++;
	};

	return SUCCESS;

};



/*********************************************/
// LOG FILE MEMORY UPLOAD PROCESS
/*********************************************/
void LOGFILE_MEMERY_LOAD()
{
	char logfile[100];
	char line[LINE_MAX];

	char tmp[60];
	char uid_sha[51];

	char *p;

	time_t	pt;
	struct	tm	lt;

	FILE *fp;

	int r;
	int _Freqidx;
	
	
	int d = 0, h = 0, m = 0, s = 0;

	time(&pt);
	localtime_r(&pt, &lt);

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...START", __FILE__, (long)getpid(), __LINE__, __func__);

	
	sprintf(logfile, "%s/%04d%02d%02d.log", DAEMON_DATA, lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);

	if( bIs_there_file(logfile) < 0)
	{
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...File Not Found(%s)", __FILE__, (long)getpid(), __LINE__, __func__, logfile);

	}else {

		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...LogFile Analysis START(%s)", __FILE__, (long)getpid(), __LINE__, __func__, logfile);

		fp = fopen(logfile, "r");

		if(fp)
		{
			while(fgets(line, LINE_MAX, fp))
			{

				if(line[0] != 't') continue;

				LogParser(line);

				//memset(line, 0x00, sizeof(line));


				if( CHECK_TIME <= _recvLOGFILE.time) break;
					
				
				pt = _recvLOGFILE.time;
				localtime_r(&pt, &lt);
				
				d = lt.tm_mday;
				h = lt.tm_hour;
				m = lt.tm_min;
				s = lt.tm_sec;
					

				// Set Memory
				set_total_memory(_recvLOGFILE.type, _recvLOGFILE.rtn, _recvLOGFILE.price, d, h, m, s);
				set_platform_memory(_recvLOGFILE.type, _recvLOGFILE.platform, _recvLOGFILE.rtn, _recvLOGFILE.price);
				set_sdk_memory(_recvLOGFILE.type, _recvLOGFILE.sdk, _recvLOGFILE.rtn, _recvLOGFILE.price);
				set_cmp_memory(_recvLOGFILE.type, _recvLOGFILE.camp_type, _recvLOGFILE.rtn, _recvLOGFILE.price);
				set_tf_memory(_recvLOGFILE.type, _recvLOGFILE.tf, _recvLOGFILE.rtn, _recvLOGFILE.price);

				if(_recvLOGFILE.camp_idx > 0) set_camp_memory(_recvLOGFILE.type, _recvLOGFILE.camp_idx, _recvLOGFILE.price);
				if(_recvLOGFILE.pub_idx > 0) set_pub_memory(_recvLOGFILE.type, _recvLOGFILE.pub_idx, _recvLOGFILE.rtn, _recvLOGFILE.price);
				if(_recvLOGFILE.ads_idx > 0) set_ads_memory(_recvLOGFILE.type, _recvLOGFILE.ads_idx, _recvLOGFILE.price);


				if(_recvLOGFILE.type == DEFINE_LOG_REQUEST)
				{
					memset(uid_sha, 0x00, sizeof(uid_sha));
					sprintf(tmp, "r%s", _recvLOGFILE.uid);
					SHA1Encode(tmp, uid_sha);
					//printf("%s\n", uid_sha);
					hash_item.key = uid_sha;
					if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
					{
						hash_item.key	= uid_sha;
						hash_item.data	= "1";//_recvLOGFILE.uid;
						hsearch(hash_item, ENTER);

						_TOTAL.u_request++;

					};
				}
				else if(_recvLOGFILE.type == DEFINE_LOG_VIEW)
				{
					memset(uid_sha, 0x00, sizeof(uid_sha));
					sprintf(tmp, "v%s", _recvLOGFILE.uid);
					SHA1Encode(tmp, uid_sha);
					hash_item.key = uid_sha;
					if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
					{
						//_Freq[_Freqcnt].imp = 1;
						//setFreqUpdate(0,1);

						hash_item.key	= uid_sha;
						hash_item.data	= "1";
						//hash_item.data	= _Freqcnt;
						hsearch(hash_item, ENTER);

						_TOTAL.u_imp++;
						//_Freqcnt++;
						//printf("%s\t%d\n", uid_sha, _Freqcnt );

					}
					/*
					else
					{
						_Freqidx = -1;
						_Freqidx = (int)(hash_result->data);
						if(_Freqidx > -1) {
							r = _Freq[_Freqidx].imp;
							_Freq[_Freqidx].imp = _Freq[_Freqidx].imp + 1;
							setFreqUpdate(r, _Freq[_Freqidx].imp);
						};
					};
					*/
				}
				else if(_recvLOGFILE.type == DEFINE_LOG_CLICK)
				{
					memset(uid_sha, 0x00, sizeof(uid_sha));

					sprintf(tmp, "c%s", _recvLOGFILE.uid);
					SHA1Encode(tmp, uid_sha);

					//printf("%s\n", uid_sha);

					hash_item.key = uid_sha;
					if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
					{
						hash_item.key	= uid_sha;
						hash_item.data	= "1";//_recvLOGFILE.uid;
						hsearch(hash_item, ENTER);

						_TOTAL.u_clk++;
					};
				};
			};

			fclose(fp);
		};

		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...LogFile Analysis END(%s)", __FILE__, (long)getpid(), __LINE__, __func__, logfile);
	};

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...END", __FILE__, (long)getpid(), __LINE__, __func__);

};



/*********************************************/
// 30Days LOG FILE MEMORY UPLOAD PROCESS
/*********************************************/
void* LAST_LOGFILE_LOAD(void* arg)
{
	
	struct thread_arg *t_arg = (struct thread_arg *)arg;
			
	int log;
	
	char last_logfile[100];
	char line[LINE_MAX];

	int i, n;
	char Line2[1024], name[20], val[1000];
	char tmp;
	char *q, *p;
	
	uchar _recv_type;
	uint _recv_time;
	uchar _recv_rtn;                                
	uint _recv_price;

	time_t	pt;
	struct	tm	lt;

	FILE *fp;

	int d = 0, h = 0, m = 0, s = 0;
	
	log = t_arg->idx;
	//log += 1;
	
	time(&pt);
	pt -= (60*60*24*log);
	localtime_r(&pt, &lt);
				                             
				                             
	memset(last_logfile, 0, sizeof(last_logfile));			                             
	sprintf(last_logfile, "%s/%04d%02d%02d.log", DAEMON_DATA, lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);		
		

	if( bIs_there_file(last_logfile) < 0)
	{
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...File Not Found(%s)", __FILE__, (long)getpid(), __LINE__, __func__, last_logfile);

	}else {
	
	
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...LogFile Analysis START(%s) thread (No:%d)", __FILE__, (long)getpid(), __LINE__, __func__, last_logfile, t_arg->idx);

		fp = fopen(last_logfile, "r");

		if(fp)
		{
			while(fgets(line, LINE_MAX, fp))
			{

				if(line[0] != 't') continue;

				p = line;				
				
				replace(p, " ", "");
				
				q = p;
				
			 	n = 0;
				while(*p != 0)
				{
					if(*p == TOKEN)
					{
						*p = ' ';
						n++;
			   		};
			   		p++;
			   	};
				n++;
				
				_recv_type = 0;
				_recv_time = 0;
				_recv_rtn = 0;
				_recv_price = 0;
			
				for(i = 0 ; i < n  ; i++)
				{
			
					memset(name, 0, sizeof(name));
			   		memset(val, 0, sizeof(val));
			
					sscanf(q, "%s", Line2);
			
					p = strchr(Line2, '=');
			   		if(p == NULL) break;
			
					*p = ' ';
			   		sscanf(Line2, "%s%s", name, val);
			
			
			   		if(strcmp(name, "type") == 0) {
			
			   			strcpy(&tmp, val);
			
			           if( tmp == 'r') 							_recv_type = DEFINE_LOG_REQUEST;
			           else if( tmp == 'v') 					_recv_type = DEFINE_LOG_VIEW;
			           else if( tmp == 'c') 					_recv_type = DEFINE_LOG_CLICK;
			           else if( tmp == 'p') 					_recv_type = DEFINE_LOG_POSTBACK;
			        }				
			        else if(strcmp(name, "time") == 0)          _recv_time = atoi(val);	
			        else if(strcmp(name, "price") == 0)         _recv_price = atoi(val);
			        else if(strcmp(name, "rtn") == 0) 			{
			            if( atoi(val) == 0 ) 					_recv_rtn = DEFINE_RTN_ISAD;
			            else 									_recv_rtn = DEFINE_RTN_NOAD;				        
			        }
			   		q = strchr(q, ' ');
					q++;
				};
				
				pt = _recv_time;
				localtime_r(&pt, &lt);
				
				d = lt.tm_mday;
				h = lt.tm_hour;
				m = lt.tm_min;
				s = lt.tm_sec;
					
				// Set Memory
				switch(_recv_type)
				{
					case DEFINE_LOG_REQUEST :
						
						_TOTAL_TIME[d].hour[h].request[m][s]++;
			
						if(_recv_rtn == DEFINE_RTN_ISAD) {
							_TOTAL_TIME[d].hour[h].response[m][s]++;
						}else {
							_TOTAL_TIME[d].hour[h].noad[m][s]++;
						}				
						break;
			
					case DEFINE_LOG_VIEW :
						_TOTAL_TIME[d].hour[h].imp[m][s]++;				
						break;
			
					case DEFINE_LOG_CLICK :
						_TOTAL_TIME[d].hour[h].clk[m][s]++;
						_TOTAL_TIME[d].hour[h].clk_cost[m][s] += _recv_price;				
						break;
						
					case DEFINE_LOG_POSTBACK :
						_TOTAL_TIME[d].hour[h].postback[m][s]++;
						break;
						
				};
			};	
			fclose(fp);
		};

		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...LogFile Analysis END(%s) thread (No:%d)", __FILE__, (long)getpid(), __LINE__, __func__, last_logfile, t_arg->idx);
	};
	
	
	//pthread_detach( pthread_self() );
	
	
};



int getCampaignName(int low, int high, int idx)
{
	int i;

	if(low > high) return -1;

	i = (low + high) / 2;

	if( _CAMP_INFO[i].idx == idx)
	{
		return i;
	}
	else if( _CAMP_INFO[i].idx < idx)
	{
		low = i + 1;
		getCampaignName(low, high, idx);
	}
	else
	{
		high = i - 1;
		getCampaignName(low, high, idx);
	};
};


int getPubName(int low, int high, int idx)
{
	int i;

	if(low > high) return -1;

	i = (low + high) / 2;

	if( _PUB_INFO[i].idx == idx)
	{
		return i;
	}
	else if( _PUB_INFO[i].idx < idx)
	{
		low = i + 1;
		getPubName(low, high, idx);
	}
	else
	{
		high = i - 1;
		getPubName(low, high, idx);
	};
};

int getAdsName(int low, int high, int idx)
{
	int i;

	if(low > high) return -1;

	i = (low + high) / 2;

	if( _ADS_INFO[i].idx == idx)
	{
		return i;
	}
	else if( _ADS_INFO[i].idx < idx)
	{
		low = i + 1;
		getAdsName(low, high, idx);
	}
	else
	{
		high = i - 1;
		getAdsName(low, high, idx);
	};
};

int getDbMemoryLoad()
{
	int i = 0;
	int j = 0;
		
	char sql[1024];

	MYSQL		mysql;
	MYSQL_RES*	res;
	MYSQL_ROW	row;

	// MySQl Init, Connect
	mysql_init(&mysql);
	if ( mysql_real_connect(&mysql, DBIP, DBID, DBPW, DBNAME , DBPORT, (char *)NULL, 0) == NULL )
	{
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return -1;
	};

	sprintf(sql, "SET NAMES utf8");
    mysql_real_query(&mysql,sql,strlen(sql));

	sprintf(sql, "SELECT campaign_idx, title, CASE status_flag WHEN 'Y' THEN 1 ELSE 0 END FROM campaign_info WHERE status_flag = 'Y' ORDER BY campaign_idx ASC");
	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...error(%s) <%s>", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql), sql);
		return -1;
	};
	res = mysql_store_result(&mysql);
	j = (int)mysql_num_rows(res);
	
	
	pthread_mutex_lock( &DB_SYNC_CMP );
	
	if(_CAMP_INFO) {
		_CAMP_INFOcnt = 0;
		free(_CAMP_INFO);
		_CAMP_INFO = NULL;
	};
		
	_CAMP_INFO = (stDBINFO *)calloc(sizeof(stDBINFO), j);
		
	for(i = 0 ; i < j ; i++)
	{
		row = mysql_fetch_row(res);
		
		_CAMP_INFO[i].idx = atoi(row[0]);
		strcpy(_CAMP_INFO[i].name,row[1]);
		_CAMP_INFO[i].status = atoi(row[2]);
	};	
	_CAMP_INFOcnt = j;	
	
	pthread_mutex_unlock( &DB_SYNC_CMP );
	
	
	mysql_free_result(res);
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s..._CAMP_INFO DB RELOAD complete", __FILE__, (long)getpid(), __LINE__, __func__);
	
	
	
	
	//Ads 
	sprintf(sql, "SELECT ads_idx, campaign_idx, IFNULL(CASE target_flag WHEN 'AD_RET' THEN 1 WHEN 'AD_NON' THEN 2 WHEN 'AD_TAG' THEN 3 END, 0), title, price, IFNULL(banner_path_1,'no'), CASE status_flag WHEN 'Y' THEN 1 ELSE 0 END FROM ads_contents WHERE status_flag = 'Y' ORDER BY ads_idx ASC");
	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...error(%s) <%s>", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql), sql);
		return -1;
	};
	res = mysql_store_result(&mysql);
	j = (int)mysql_num_rows(res);
	
	pthread_mutex_lock( &DB_SYNC_ADS );
	
	if(_ADS_INFO) {
		free(_ADS_INFO);
		_ADS_INFO = NULL;
		_ADS_INFOcnt = 0;
	};
	
	_ADS_INFO = (stDBINFO2 *)calloc(sizeof(stDBINFO2), j);
	for(i = 0 ; i < j ; i++)
	{
		row = mysql_fetch_row(res);

		_ADS_INFO[i].idx = atoi(row[0]);
		_ADS_INFO[i].camp_idx = atoi(row[1]);
		_ADS_INFO[i].type = atoi(row[2]);
		strcpy(_ADS_INFO[i].name, row[3]);
		_ADS_INFO[i].price = atoi(row[4]);
		strcpy(_ADS_INFO[i].img, row[5]);
		//_ADS_INFO[i].status = atoi(row[6]);

	};
	_ADS_INFOcnt = j;
	
	pthread_mutex_unlock( &DB_SYNC_ADS );
	
	mysql_free_result(res);
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s..._ADS_INFO DB LOAD complete", __FILE__, (long)getpid(), __LINE__, __func__);
	
	
	
	
	sprintf(sql, "SELECT idx, name FROM pub_info ORDER BY idx ASC");
    	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...error(%s) <%s>", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql), sql);
		return -1;
	};
	res = mysql_store_result(&mysql);
	j = (int)mysql_num_rows(res);
	
	pthread_mutex_lock( &DB_SYNC_PUB );
	
	if(_PUB_INFO) {
		free(_PUB_INFO);
		_PUB_INFO = NULL;
		_PUB_INFOcnt = 0;
	};	
	_PUB_INFO = (stDBINFO *)calloc(sizeof(stDBINFO), j);
	for(i = 0 ; i < j ; i++)
	{
		row = mysql_fetch_row(res);
		_PUB_INFO[i].idx = atoi(row[0]);
		strcpy(_PUB_INFO[i].name, row[1]);
	};
	_PUB_INFOcnt = j;
	
	pthread_mutex_unlock( &DB_SYNC_PUB );
	
	
	mysql_free_result(res);
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s..._PUB_INFO DB LOAD complete", __FILE__, (long)getpid(), __LINE__, __func__);

	
	

	mysql_close(&mysql);

};


/*********************************************/
// CORE_DB_LOAD_PROCESS
/*********************************************/
void* CORE_DB_LOAD_PROCESS()
{
	int i;

	time_t	pt;
	struct	tm	lt;
	
	FILE *fp;

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...START", __FILE__, (long)getpid(), __LINE__, __func__);

	
	while(ON)
	{
		
		//DB Memory LOAD
		getDbMemoryLoad();


		fp = fopen(CMP_MEMORY_FILE, "w");
		if(fp)
		{
			time(&pt);
			localtime_r(&pt, &lt);

			for(i = 0 ; i < _CAMP_INFOcnt ; i++)
			{
				fprintf(fp, "%d\t%s\n",_CAMP_INFO[i].idx,_CAMP_INFO[i].name);
			};

			fprintf(fp, "[%04d/%02d/%02d %02d:%02d:%02d]\tTotal Campaign : %d\n", lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, _CAMP_INFOcnt);
			fflush(fp);
			fclose(fp);	
		};
		
		sleep(60*10);
		
	};

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...END", __FILE__, (long)getpid(), __LINE__, __func__);

};





/*********************************************/
// CORE_RTS_TOTAL_PROCESS
/*********************************************/
void* CORE_RTS_TOTAL_PROCESS()
{
	int i;
	int cntLen;
	int Result;

	char recvPacket[10];
	char SendPacket[1024*10];

	struct sockaddr_in cntAddr;

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);


	int sock = UDPSock_SERVER(sysInfo.RTS_REPORT1_PORT);

	while(ON)
	{

		SendPacket[0] = 0;
		recvPacket[0] = 0;
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(sock, recvPacket, sizeof recvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		if(Result < 0) continue;
		recvPacket[Result] = 0;

  		sprintf(CodePlusPacket,"{\"TOTAL\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u, \"u_request\":%u, \"u_imp\":%u, \"u_clk\":%u}, ", _TOTAL.request,_TOTAL.response,_TOTAL.noad,_TOTAL.imp,_TOTAL.clk,_TOTAL.clk_cost,_TOTAL.postback, _TOTAL.u_request, _TOTAL.u_imp, _TOTAL.u_clk);
  		sprintf(CodePlusPacket,"\"PLATFORM_ANDROID\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _PLATFORM_ANDROID.request,_PLATFORM_ANDROID.response,_PLATFORM_ANDROID.noad,_PLATFORM_ANDROID.imp,_PLATFORM_ANDROID.clk,_PLATFORM_ANDROID.clk_cost,_PLATFORM_ANDROID.postback);
  		sprintf(CodePlusPacket,"\"PLATFORM_IOS\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _PLATFORM_IOS.request,_PLATFORM_IOS.response,_PLATFORM_IOS.noad,_PLATFORM_IOS.imp,_PLATFORM_IOS.clk,_PLATFORM_IOS.clk_cost,_PLATFORM_IOS.postback);
  		sprintf(CodePlusPacket,"\"PLATFORM_ETC\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _PLATFORM_ETC.request,_PLATFORM_ETC.response,_PLATFORM_ETC.noad,_PLATFORM_ETC.imp,_PLATFORM_ETC.clk,_PLATFORM_ETC.clk_cost,_PLATFORM_ETC.postback);
  		sprintf(CodePlusPacket,"\"SDK_APP\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _SDK_APP.request,_SDK_APP.response,_SDK_APP.noad,_SDK_APP.imp,_SDK_APP.clk,_SDK_APP.clk_cost,_SDK_APP.postback);
  		sprintf(CodePlusPacket,"\"SDK_WEB\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _SDK_WEB.request,_SDK_WEB.response,_SDK_WEB.noad,_SDK_WEB.imp,_SDK_WEB.clk,_SDK_WEB.clk_cost,_SDK_WEB.postback);
  		sprintf(CodePlusPacket,"\"SDK_ETC\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _SDK_ETC.request,_SDK_ETC.response,_SDK_ETC.noad,_SDK_ETC.imp,_SDK_ETC.clk,_SDK_ETC.clk_cost,_SDK_ETC.postback);
  		sprintf(CodePlusPacket,"\"CPM\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _CMP_CPM.request,_CMP_CPM.response,_CMP_CPM.noad,_CMP_CPM.imp,_CMP_CPM.clk,_CMP_CPM.clk_cost,_CMP_CPM.postback);
  		sprintf(CodePlusPacket,"\"CPC\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _CMP_CPC.request,_CMP_CPC.response,_CMP_CPC.noad,_CMP_CPC.imp,_CMP_CPC.clk,_CMP_CPC.clk_cost,_CMP_CPC.postback);
  		sprintf(CodePlusPacket,"\"CPI\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _CMP_CPI.request,_CMP_CPI.response,_CMP_CPI.noad,_CMP_CPI.imp,_CMP_CPI.clk,_CMP_CPI.clk_cost,_CMP_CPI.postback);
  		sprintf(CodePlusPacket,"\"RETARGET\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}, ", _TF_TAR.request,_TF_TAR.response,_TF_TAR.noad,_TF_TAR.imp,_TF_TAR.clk,_TF_TAR.clk_cost,_TF_TAR.postback);
		sprintf(CodePlusPacket,"\"NONTARGET\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},", _TF_NON.request,_TF_NON.response,_TF_NON.noad,_TF_NON.imp,_TF_NON.clk,_TF_NON.clk_cost,_TF_NON.postback);
		sprintf(CodePlusPacket,"\"TARGET\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}}", _TF_TAG.request,_TF_TAG.response,_TF_TAG.noad,_TF_TAG.imp,_TF_TAG.clk,_TF_TAG.clk_cost,_TF_TAG.postback);

		sendto(sock, SendPacket, strlen(SendPacket)+1, 0, (struct sockaddr *)&cntAddr, sizeof(struct sockaddr));

	}


	close(sock);


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);


};




/*********************************************/
// CORE_RTS_CAMP_PROCESS
/*********************************************/
void* CORE_RTS_CAMP_PROCESS()
{
	int i;
	int idx;
	int cntLen;
	int Result;
	
	int ret;

	char *p;

	char recvPacket[10];
	char SendPacket[1024*10];

	struct sockaddr_in cntAddr;

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);


	int sock = UDPSock_SERVER(sysInfo.RTS_REPORT2_PORT);

	while(ON)
	{
		i = 0;
		SendPacket[0] = 0;
		recvPacket[0] = 0;
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(sock, recvPacket, sizeof recvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		if(Result < 0) continue;
		recvPacket[Result] = 0;

		i = atoi(recvPacket);
		if( i >= RTS_MEM_LIMIT) continue;

		sprintf(CodePlusPacket,"[");

		ret = pthread_mutex_trylock( &DB_SYNC_CMP );
	
		if( ret == 0 )
		{			
			if(i == 0)
			{
				for( i = 1 ; i < RTS_MEM_LIMIT ; i++ )
				{
					if(_CMP[i].chk == 0) continue;
	
					if( _CMP[i].imp > 0 || _CMP[i].clk > 0) {
						
						idx = -1;
						if(_CAMP_INFOcnt > 0) idx = getCampaignName(0,_CAMP_INFOcnt,i);						
						if(idx < 0) continue;
	
						sprintf(CodePlusPacket,"{\"idx\":%d, \"name\":\"%s\", \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},",
								i, _CAMP_INFO[idx].name, _CMP[i].imp,_CMP[i].clk,_CMP[i].clk_cost,_CMP[i].postback);
					};
				};
			}else {
				if( _CMP[i].imp > 0 || _CMP[i].clk > 0) {
					idx = getCampaignName(0,_CAMP_INFOcnt,i);
					sprintf(CodePlusPacket,"{\"idx\":%d, \"name\":\"%s\", \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},",
							i, _CAMP_INFO[idx].name, _CMP[i].imp,_CMP[i].clk,_CMP[i].clk_cost,_CMP[i].postback);
				};
			};
			
			pthread_mutex_unlock( &DB_SYNC_CMP );
			
			p = strrchr(SendPacket, ',');
			*p = 0;
		};

		sprintf(CodePlusPacket,"]");

		sendto(sock, SendPacket, strlen(SendPacket)+1, 0, (struct sockaddr *)&cntAddr, sizeof(struct sockaddr));

	}

	close(sock);


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);


};


/*********************************************/
// CORE_RTS_PUB_PROCESS
/*********************************************/
void* CORE_RTS_PUB_PROCESS()
{
	int i;
	int idx;
	int cntLen;
	int Result;
	
	int ret;

	char *p;

	char recvPacket[10];
	char SendPacket[1024*10];

	struct sockaddr_in cntAddr;

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);


	int sock = UDPSock_SERVER(sysInfo.RTS_REPORT3_PORT);

	while(ON)
	{
		i = 0;
		SendPacket[0] = 0;
		recvPacket[0] = 0;
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(sock, recvPacket, sizeof recvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		if(Result < 0) continue;
		recvPacket[Result] = 0;

		i = atoi(recvPacket);
        if( i >= RTS_MEM_LIMIT) continue;

		sprintf(CodePlusPacket,"[");
	
		ret = pthread_mutex_trylock( &DB_SYNC_PUB );
	
		if( ret == 0 )
		{				
			if(i == 0)
			{
				for( i = 1 ; i < RTS_MEM_LIMIT ; i++ )
				{
					if(_PUB[i].chk == 0) continue;
	
					if( _PUB[i].request > 0) {
						
						idx = -1;
						if(_PUB_INFOcnt > 0) idx = getCampaignName(0,_PUB_INFOcnt,i);
							
						idx = getPubName(0,_PUB_INFOcnt, i);
	
						if(idx < 0) continue;
	
						sprintf(CodePlusPacket,"{\"idx\":%d, \"name\":\"%s\", \"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},",
								i, _PUB_INFO[idx].name, _PUB[i].request,_PUB[i].response,_PUB[i].noad,_PUB[i].imp,_PUB[i].clk,_PUB[i].clk_cost,_PUB[i].postback);
	
					};
				};
	        }else {
	
				idx = getPubName(0,_PUB_INFOcnt, i);
				sprintf(CodePlusPacket,"{\"idx\":%d, \"name\":\"%s\", \"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},",
						i, _PUB_INFO[idx].name, _PUB[i].request,_PUB[i].response,_PUB[i].noad,_PUB[i].imp,_PUB[i].clk,_PUB[i].clk_cost,_PUB[i].postback);
	        }
	        
	        pthread_mutex_unlock( &DB_SYNC_PUB );
	
			p = strrchr(SendPacket, ',');
			*p = 0;
			
		};

		sprintf(CodePlusPacket,"]");

		sendto(sock, SendPacket, strlen(SendPacket)+1, 0, (struct sockaddr *)&cntAddr, sizeof(struct sockaddr));

		//fprintf(stderr,"{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u}\n", _PUB[4].request,_PUB[4].response,_PUB[4].noad,_PUB[4].imp,_PUB[4].clk,_PUB[4].clk_cost,_PUB[4].postback);

	}

	close(sock);


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);


};



/*********************************************/
// CORE_RTS_ADS_PROCESS
/*********************************************/
void* CORE_RTS_ADS_PROCESS()
{
	int i;
	int idx;
	int cntLen;
	int Result;
	
	int ret;

	char *p;

	char recvPacket[10];
	char SendPacket[1024*1024];

	struct sockaddr_in cntAddr;

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);

	int sock = UDPSock_SERVER(sysInfo.RTS_REPORT4_PORT);

	while(ON)
	{
		i = 0;
		idx = 0;
		SendPacket[0] = 0;
		recvPacket[0] = 0;
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(sock, recvPacket, sizeof recvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		if(Result < 0) continue;
		recvPacket[Result] = 0;

		i = atoi(recvPacket);

		if( i >= RTS_MEM_LIMIT) continue;

		sprintf(CodePlusPacket,"[");

		ret = pthread_mutex_trylock( &DB_SYNC_ADS );
	
		if( ret == 0 )
		{
			if(i == 0)
			{
	
				for( i = 0 ; i < _ADS_INFOcnt ; i++ )
				{
					if( _ADS[_ADS_INFO[i].idx].chk == 0 ) continue;
	
					if( _ADS[_ADS_INFO[i].idx].imp > 0 || _ADS[_ADS_INFO[i].idx].clk > 0 ) {
	
						sprintf(CodePlusPacket,"{\"idx\":%d, \"name\":\"%s\", \"img\":\"%s\", \"type\":%d, \"price\":%d, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},",
							_ADS_INFO[i].idx, _ADS_INFO[i].name, _ADS_INFO[i].img, _ADS_INFO[i].type, _ADS_INFO[i].price, _ADS[_ADS_INFO[i].idx].imp,_ADS[_ADS_INFO[i].idx].clk,_ADS[_ADS_INFO[i].idx].clk_cost,_ADS[_ADS_INFO[i].idx].postback);
	
					};
				}
			}
			else
			{
				idx = -1;
				if(_ADS_INFOcnt > 0) idx = getAdsName(0,_ADS_INFOcnt,i);
					
				if(idx > 0) {
					sprintf(CodePlusPacket,"{\"idx\":%d, \"name\":\"%s\", \"img\":\"%s\", \"type\":%d, \"price\":%d, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u},",
						i, _ADS_INFO[idx].name, _ADS_INFO[idx].img, _ADS_INFO[idx].type, _ADS_INFO[idx].price, _ADS[i].imp,_ADS[i].clk,_ADS[i].clk_cost,_ADS[i].postback);
				};
			}
			
			pthread_mutex_unlock( &DB_SYNC_ADS );
	
			p = strrchr(SendPacket, ',');
			*p = 0;
			
		};

		sprintf(CodePlusPacket,"]");

		//printf("%s\n", SendPacket);


		sendto(sock, SendPacket, strlen(SendPacket)+1, 0, (struct sockaddr *)&cntAddr, sizeof(struct sockaddr));

	}

	close(sock);


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);


};


/*********************************************/
// CORE_RTS_LAST_LOAD_PROCESS
/*********************************************/
void* CORE_RTS_LAST_LOAD_PROCESS()
{
	int i;	
        
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);


	int min = 1;           
	int max = 8;

	while(ON)
	{
		memset(thr_arg, 0x00, sizeof(struct thread_arg) * _MAX_MONTH_MEMORY);

		for(i = min ; i < max ; i++)
		{
			thr_arg[i].idx = i;
			if( pthread_create(&thr_arg[i].pt_id, NULL, LAST_LOGFILE_LOAD, (void *)&thr_arg[i]) ) {
				LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
			};

			sleep(1);
		};

		for(i = min ; i < max ; i++)
		{
			pthread_join(thr_arg[i].pt_id, NULL);
			//pthread_detach(thr_arg[i].pt_id);
		}

		min = max;
		max = max + 7;

		if(max > 10) break;

	};
	
	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);
	   
	    
	pthread_detach( pthread_self() );

};


/*********************************************/
// CORE_RTS_TOTAL_TIME_PROCESS
/*********************************************/
void* CORE_RTS_TOTAL_TIME_PROCESS()
{
	int i;
	int cntLen;
	int Result;
	
	time_t	pt;
	struct	tm	lt;

	char recvPacket[10];
	char SendPacket[1024*10];

	struct sockaddr_in cntAddr;

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...Start", __FILE__, (long)getpid(), __LINE__, __func__);


	int sock = UDPSock_SERVER(sysInfo.RTS_REPORT5_PORT);
	
	
	int d = 0, h = 0, m = 0, s = 0;
	int hour = 0, min = 0, sec = 0;	
	
	uint request;
	uint response;
	uint noad;
	uint imp;
	uint clk;
	uint clk_cost;
	uint postback;
	
	
	while(ON)
	{
		d = 0;
		request = 0;
		response = 0;
		noad = 0;
		imp = 0;
		clk = 0;
		clk_cost = 0;
		postback =0;
		
		SendPacket[0] = 0;
		recvPacket[0] = 0;
		cntLen = sizeof(struct sockaddr);
		Result = recvfrom(sock, recvPacket, sizeof recvPacket, 0, (struct sockaddr *)&cntAddr, &cntLen);
		if(Result < 0) continue;
		recvPacket[Result] = 0;
		
		d = atoi(recvPacket);		
		if(d > 31) continue;
  		
  		time(&pt);
		localtime_r(&pt, &lt);
		
		if(d == 0) d = lt.tm_mday;
  		
  		hour = lt.tm_hour;
  		min = lt.tm_min;
  		sec = lt.tm_sec;
  		
		for(h = 0 ; h <= hour ; h++)
		{
			if(h == hour)
			{
				for(m = 0 ; m <= min ; m++)
				{
					if( m == min)
					{
						for(s = 0 ; s <= sec ; s++)
						{
							request += _TOTAL_TIME[d].hour[h].request[m][s];
							response += _TOTAL_TIME[d].hour[h].response[m][s];
							noad += _TOTAL_TIME[d].hour[h].noad[m][s];
							imp += _TOTAL_TIME[d].hour[h].imp[m][s];
							clk += _TOTAL_TIME[d].hour[h].clk[m][s];
							clk_cost += _TOTAL_TIME[d].hour[h].clk_cost[m][s];
							postback += _TOTAL_TIME[d].hour[h].postback[m][s];
						};
					}else {
						for(s = 0 ; s < 60 ; s++)
						{
							request += _TOTAL_TIME[d].hour[h].request[m][s];
							response += _TOTAL_TIME[d].hour[h].response[m][s];
							noad += _TOTAL_TIME[d].hour[h].noad[m][s];
							imp += _TOTAL_TIME[d].hour[h].imp[m][s];
							clk += _TOTAL_TIME[d].hour[h].clk[m][s];
							clk_cost += _TOTAL_TIME[d].hour[h].clk_cost[m][s];
							postback += _TOTAL_TIME[d].hour[h].postback[m][s];							
						};
					};
  				};
			}else {
  				for(m = 0 ; m < 60 ; m++)
  				{
					for(s = 0 ; s < 60 ; s++)
					{
						request += _TOTAL_TIME[d].hour[h].request[m][s];
						response += _TOTAL_TIME[d].hour[h].response[m][s];
						noad += _TOTAL_TIME[d].hour[h].noad[m][s];
						imp += _TOTAL_TIME[d].hour[h].imp[m][s];
						clk += _TOTAL_TIME[d].hour[h].clk[m][s];
						clk_cost += _TOTAL_TIME[d].hour[h].clk_cost[m][s];
						postback += _TOTAL_TIME[d].hour[h].postback[m][s];						
					};
				};
			};
		};
		
  		sprintf(CodePlusPacket,"{\"TOTAL\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u }}"
  								, request, response, noad, imp, clk, clk_cost, postback);
  		
		sendto(sock, SendPacket, strlen(SendPacket)+1, 0, (struct sockaddr *)&cntAddr, sizeof(struct sockaddr));

	}

	close(sock);


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...End", __FILE__, (long)getpid(), __LINE__, __func__);


};



/*********************************************/
// CORE_INIT_PROCESS
/*********************************************/
void* CORE_INIT_PROCESS()
{
	int i;
	time_t	pt;
	struct	tm	lt;
	
	char rtxt[1024];

	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...START", __FILE__, (long)getpid(), __LINE__, __func__);

	
	// reloading
	LOGFILE_MEMERY_LOAD();
	

	while(ON)
	{
        if ( sysInfo.recvTerm == ON )
        {
            LogPrint(sysInfo.DEBUGMODE, __FILE__,"%s[%ld]{%d} %s...receive signal=SigTERM", __FILE__, (long)getpid(), __LINE__, __func__);
            break;
        };


		time(&pt);
		localtime_r(&pt, &lt);


		if( FLAG_INIT && lt.tm_hour == 0 && lt.tm_min == 0 && lt.tm_sec == 0)
		{
			FLAG_INIT = false;


			sprintf(rtxt,"{\"TOTAL\":{\"request\":%u, \"response\":%u, \"noad\":%u, \"imp\":%u, \"clk\":%u, \"clk_cost\":%u, \"postback\":%u, \"u_request\":%u, \"u_imp\":%u, \"u_clk\":%u}, ", _TOTAL.request,_TOTAL.response,_TOTAL.noad,_TOTAL.imp,_TOTAL.clk,_TOTAL.clk_cost,_TOTAL.postback, _TOTAL.u_request, _TOTAL.u_imp, _TOTAL.u_clk);
            LogPrint(sysInfo.DEBUGMODE, __FILE__,"%s", rtxt);
            
            
            time(&pt);
			pt -= (60*60*24*30);
			localtime_r(&pt, &lt);			
			memset(&_TOTAL_TIME[lt.tm_mday], 0, sizeof(stTOTAL_RTS_TIME_MONTH));
			
			memset(&_TOTAL, 0, sizeof(stTOTAL_RTS));
			memset(&_PLATFORM_ANDROID, 0, sizeof(stTOTAL_RTS));
			memset(&_PLATFORM_IOS, 0, sizeof(stTOTAL_RTS));
			memset(&_PLATFORM_ETC, 0, sizeof(stTOTAL_RTS));
			memset(&_SDK_APP, 0, sizeof(stTOTAL_RTS));
			memset(&_SDK_WEB, 0, sizeof(stTOTAL_RTS));
			memset(&_SDK_ETC, 0, sizeof(stTOTAL_RTS));
			memset(&_CMP_CPM, 0, sizeof(stTOTAL_RTS));
			memset(&_CMP_CPC, 0, sizeof(stTOTAL_RTS));
			memset(&_CMP_CPI, 0, sizeof(stTOTAL_RTS));
			memset(&_TF_TAR, 0, sizeof(stTOTAL_RTS));
			memset(&_TF_NON, 0, sizeof(stTOTAL_RTS));
			memset(&_TF_TAG, 0, sizeof(stTOTAL_RTS));
			memset(&_CMP, 0, sizeof(stTOTAL_RTS)*RTS_MEM_LIMIT);
            memset(&_PUB, 0, sizeof(stTOTAL_RTS)*RTS_MEM_LIMIT);
            memset(&_ADS, 0, sizeof(stTOTAL_RTS)*RTS_MEM_LIMIT);
            //memset(&_Freq, 0, sizeof(stFREQ)*FREQ_MEM_LIMIT);

            //for( i = 0 ; i < 51 ; i++) _FreqRTS[i] = 0;

			hdestroy();
			hcreate(MEMORY_LIMIT_CNT);

			FLAG_INIT = true;

			LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...RESET MEMORY", __FILE__, (long)getpid(), __LINE__, __func__);

			sleep(1);
		}
		else
		{
			FLAG_INIT = true;
		};

		usleep(5);
	};


	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...END", __FILE__, (long)getpid(), __LINE__, __func__);

};



/* ================================================================================================
 *  main
 * ============================================================================================= */
int main(int argc, char *argv[])
{
    int pthread_result;
	pthread_t thread0, thread1, thread2, thread3, thread4;
	pthread_t thread5, thread6, thread7, thread8;
 

	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SigHandler);

	MainInitConf();


    bPut_pid_to_file(RTS_PROC_FILE_NAME_PID, getpid());


    LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...ENGINE START", __FILE__, (long)getpid(), __LINE__, __func__);


	memset(&_TOTAL_TIME, 0, sizeof(stTOTAL_RTS_TIME_MONTH) * _MAX_MONTH_MEMORY);
		
	memset(&_TOTAL, 0, sizeof(stTOTAL_RTS));
	memset(&_PLATFORM_ANDROID, 0, sizeof(stTOTAL_RTS));
	memset(&_PLATFORM_IOS, 0, sizeof(stTOTAL_RTS));
	memset(&_PLATFORM_ETC, 0, sizeof(stTOTAL_RTS));
	memset(&_SDK_APP, 0, sizeof(stTOTAL_RTS));
	memset(&_SDK_WEB, 0, sizeof(stTOTAL_RTS));
	memset(&_SDK_ETC, 0, sizeof(stTOTAL_RTS));
	memset(&_CMP_CPM, 0, sizeof(stTOTAL_RTS));
	memset(&_CMP_CPC, 0, sizeof(stTOTAL_RTS));
	memset(&_CMP_CPI, 0, sizeof(stTOTAL_RTS));
	memset(&_TF_TAR, 0, sizeof(stTOTAL_RTS));
	memset(&_TF_NON, 0, sizeof(stTOTAL_RTS));
	memset(&_TF_TAG, 0, sizeof(stTOTAL_RTS));
	memset(&_CMP, 0, sizeof(stTOTAL_RTS)*RTS_MEM_LIMIT);
	memset(&_PUB, 0, sizeof(stTOTAL_RTS)*RTS_MEM_LIMIT);
	memset(&_ADS, 0, sizeof(stTOTAL_RTS)*RTS_MEM_LIMIT);
	//memset(&_Freq, 0, sizeof(stFREQ)*FREQ_MEM_LIMIT);

	//memset(_FreqRTS, 0, sizeof(_FreqRTS));


	hcreate(MEMORY_LIMIT_CNT);


	if((pthread_result = pthread_create( &thread0, NULL, CORE_RCV_PROCESS, NULL))) {
    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RCV_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
    };

    if((pthread_result = pthread_create( &thread1, NULL, CORE_DB_LOAD_PROCESS, NULL))) {
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_DB_LOAD_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
	};
    
	if((pthread_result = pthread_create( &thread2, NULL, CORE_INIT_PROCESS, NULL))) {
    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_INIT_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
    };    
	
    if((pthread_result = pthread_create( &thread3, NULL, CORE_RTS_TOTAL_PROCESS, NULL))) {
    	LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RTS_TOTAL_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
    };

    if((pthread_result = pthread_create( &thread4, NULL, CORE_RTS_CAMP_PROCESS, NULL))) {
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RTS_CAMP_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
	};

	if((pthread_result = pthread_create( &thread5, NULL, CORE_RTS_PUB_PROCESS, NULL))) {
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RTS_PUB_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
	};

	if((pthread_result = pthread_create( &thread6, NULL, CORE_RTS_ADS_PROCESS, NULL))) {
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RTS_ADS_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
	};
	
	if((pthread_result = pthread_create( &thread7, NULL, CORE_RTS_TOTAL_TIME_PROCESS, NULL))) {
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RTS_TOTAL_TIME_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
	};
	
	if((pthread_result = pthread_create( &thread8, NULL, CORE_RTS_LAST_LOAD_PROCESS, NULL))) {
		LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...CORE_RTS_LAST_LOAD_PROCESS creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
	};
	

	pthread_join( thread0, NULL);
	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);
	pthread_join( thread3, NULL);
	pthread_join( thread4, NULL);
	pthread_join( thread5, NULL);
	pthread_join( thread6, NULL);
	pthread_join( thread7, NULL);
	pthread_join( thread8, NULL);
	
	
  	hdestroy();


  	if(_CAMP_INFO) free(_CAMP_INFO);
  	if(_PUB_INFO) free(_PUB_INFO);
	if(_ADS_INFO) free(_ADS_INFO);

	remove(RTS_PROC_FILE_NAME_PID);	
		
  	
    LogPrint(sysInfo.DEBUGMODE, __FILE__, "%s[%ld]{%d} %s...ENGINE END", __FILE__, (long)getpid(), __LINE__, __func__);
    
    
    return 0;
    
};

