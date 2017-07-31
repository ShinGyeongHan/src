/* ====================================================================
 *
 *  Company     : Copyright ⓒ 2015. All Rights Reserved.
 *  Author      : zakorea@gmail.com
 *	
 ====================================================================*/
 

 
/*********************************************/
// definition
/*********************************************/

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif


#include "common.h"

#include <mysql.h>                  
#include <pthread.h>
#include <search.h>                 


extern SysInfo sysInfo;


char LogDate[20];
char LoadFileName[100];

bool MANUAL = false;
uchar LOGSHOW;
bool PROCESS = false;

uint _LINE_COUNT = 0;


/*********************************************
//	Hash Memory Setup
/*********************************************/
ENTRY    hash_item;
ENTRY    *hash_result;


//로그버퍼
typedef struct {    
    uchar type;
    uchar type2;
    uchar type3;
    
    uchar ads_user_type;
    uchar user_auto_share;
    
    uint uidx;
    uint ads_info_idx;
    uint fb_ads_idx;
    uint feed_idx;
    uint able_cnt;
    
    uint cost_adv;
    uint cost_user;
    uint gap_like;
    uint audience;
    
    uchar lncid[30];
    uchar userip[16];    
        	
}logBUFFER;

//로그버퍼
logBUFFER	_recvLOG;

//TOTAL
typedef struct {
    uint user_join;
    uint user_join_sum;
    uint user_del;
    
    uint user_friends;
    uint user_friends_sum; 
       
    uint user_auto_share_Y;
    uint user_auto_share_S;
    uint user_auto_share_N;    
    
    uint user_login_unique;
    uint user_login_web;
    uint user_login_aos;
    uint user_login_ios;
    
    uint fb_ads_count;
    
    uint fb_feed_share_auto;
    uint fb_feed_share_self;
    
    uint click;
    uint uclick;
        
    uint total_like_sum;
    uint total_like_count; 
    uint total_like_cost_adv;
    uint total_like_cost_user;
    uint total_cpc_click;
    uint total_cpc_cost_adv;
    uint total_cpc_cost_user;
    uint total_ncpc_click;
    uint total_ncpc_cost_adv;
    uint total_ncpc_cost_user;    
    uint total_cpi_count;
    uint total_cpi_cost_adv;
    uint total_cpi_cost_user;
    uint total_cps_count;
    uint total_cps_cost;
    uint total_cpa_count;
    uint total_cpa_cost;
    
}stTOTAL;
stTOTAL	_TOTAL;



//ADS_INFO Nlog
typedef struct {
	uchar chk;
	uint ads_info_idx;
    uint click;
    uint uclick;
}stNLOG_ADSINFO;
stNLOG_ADSINFO*		_ADSINFO = NULL;
int _ADSINFOcnt = 0;


//_FB_ADS
typedef struct {
	uchar chk;
    uint fb_ads_idx;
    uint fb_feed_share_auto;
    uint fb_feed_share_self;
    uint fb_feed_audience;
    uint click;
    uint uclick;
    uint total_like_sum;
    uint total_like_count;
    uint total_like_cost_adv;
    uint total_like_cost_user;
    uint total_cpc_click;
    uint total_cpc_cost_adv;
    uint total_cpc_cost_user;
    uint total_ncpc_click;
    uint total_ncpc_cost_adv;
    uint total_ncpc_cost_user;    
    uint total_cpi_count;
    uint total_cpi_cost_adv;
    uint total_cpi_cost_user;
    uint total_cps_count;
    uint total_cps_cost;
    uint total_cpa_count;
    uint total_cpa_cost;
}stNLOG_FB_ADS;

//_FB_ADS
stNLOG_FB_ADS* _FB_ADS = NULL;
int _FB_ADScnt = 0;



/*
//fb_feed
typedef struct {
    uint feed_idx;
    uint clk;
    uint uclk;
    uint install;
    uint cost;
}stNLOG_FB_FEED;
stNLOG_FB_FEED*	FB_FEED = NULL;
int FB_FEEDcnt = 0;
*/



/**********************************************/
// ProcessBAR
/**********************************************/
void* ProcessBAR()
{
	int i = 1;
	uint FILE_LINE_COUNT = 0;
	double x;
	
	char cmd[100];
	char result[100];
	char *p;
	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...Start\t%s", __FILE__, (long)getpid(), __LINE__, __func__, LoadFileName);
	
	// 결과값에서 작업파일의 레코드 건수 정보 추출
  	sprintf(cmd,"/usr/bin/wc -l %s", LoadFileName);
  	memset(result, 0x00, sizeof(result));
    SystemCommandExcute(cmd, result);
	p = strtok(result, " ");
    FILE_LINE_COUNT = atoi(p);
    result[0] = '\0';
    
    
	while(1)
	{
		
		if(PROCESS) break;
			
		if(i == 15) {
			
			checkTime(result);
			x = ( (double)_LINE_COUNT / (double)FILE_LINE_COUNT ) * 100;
			
			if(MANUAL) fprintf(stderr,"%s => TOTAL:%d\tNOW:%d\t%.1f%%\n", result, FILE_LINE_COUNT, _LINE_COUNT, x);
			
			i = 0;
			
		}else {
			if(MANUAL) fprintf(stderr,".");
		};
			
		i++;
		
		sleep(1);
		
	};
	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...End\t%s", __FILE__, (long)getpid(), __LINE__, __func__, LoadFileName);
};



/*********************************************/
// Log Line => Parsing
/*********************************************/
int LineParser(char* p)
{
	int i, n;
	char* m;
	char Line[1500], name[100], val[1024];
	char tmp;

	memset(&_recvLOG, 0, sizeof(_recvLOG));

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

			if( tmp == 'C') 						_recvLOG.type = DEFINE_LOG_CLICK;
			else if( tmp == 'T') 					_recvLOG.type = DEFINE_LOG_NCLICK;
			else if( tmp == 'K') 					_recvLOG.type = DEFINE_LOG_LIKE;
			else if( tmp == 'P') 					_recvLOG.type = DEFINE_LOG_POSTBACK;
			else if( tmp == 'J') 					_recvLOG.type = DEFINE_LOG_JOIN;
			else if( tmp == 'L') 					_recvLOG.type = DEFINE_LOG_LOGIN;
			else if( tmp == 'V') 					_recvLOG.type = DEFINE_LOG_VALID;
				
        }        
        
        else if(strcmp(name, "type2") == 0) {
			
			if( strcmp(val, "install") == 0)		_recvLOG.type2 = DEFINE_TYPE_INSTALL;
			else if( strcmp(val, "INSTALL") == 0)	_recvLOG.type2 = DEFINE_TYPE_INSTALL;
			else if( strcmp(val, "postback") == 0)	_recvLOG.type2 = DEFINE_TYPE_POSTBACK;	
			else if( strcmp(val, "cancel") == 0)	_recvLOG.type2 = DEFINE_TYPE_CANCEL;
			else if( strcmp(val, "login") == 0)		_recvLOG.type2 = DEFINE_TYPE_LOGIN;
			else if( strcmp(val, "abusing") == 0)	_recvLOG.type2 = DEFINE_TYPE_ABUSING;
			else if( strcmp(val, "cpi") == 0)		_recvLOG.type2 = DEFINE_TYPE_CPI;
			else if( strcmp(val, "cpl") == 0)		_recvLOG.type2 = DEFINE_TYPE_CPL;
			else if( strcmp(val, "like") == 0)		_recvLOG.type2 = DEFINE_TYPE_CPL;
			else if( strcmp(val, "cpa") == 0)		_recvLOG.type2 = DEFINE_TYPE_CPA;
			else if( strcmp(val, "cpc") == 0)		_recvLOG.type2 = DEFINE_TYPE_CPC;
			else if( strcmp(val, "share") == 0)		_recvLOG.type2 = DEFINE_TYPE_SHARE;
        	        
		}
		
		else if(strcmp(name, "type3") == 0) {
			
			if( strcmp(val, "auto") == 0)			_recvLOG.type3 = DEFINE_TYPE3_SHARE_AUTO;
			else if( strcmp(val, "self") == 0)		_recvLOG.type3 = DEFINE_TYPE3_SHARE_SELF;
			else if( strcmp(val, "facebook") == 0)	_recvLOG.type3 = DEFINE_TYPE3_CANCEL_FACEBOOK;				
			else if( strcmp(val, "web") == 0)		_recvLOG.type3 = DEFINE_TYPE3_LOGIN_WEB;
			else if( strcmp(val, "AOS") == 0)		_recvLOG.type3 = DEFINE_TYPE3_LOGIN_AOS;
			else if( strcmp(val, "IOS") == 0)		_recvLOG.type3 = DEFINE_TYPE3_LOGIN_IOS;
			else if( strcmp(val, "ncost") == 0)	_recvLOG.type3 = DEFINE_TYPE3_NCLICK;
		}
		
		else if(strcmp(name, "ads_user_type") == 0) {
			
			if( strcmp(val, "CPI") == 0)			_recvLOG.ads_user_type = DEFINE_ADSTYPE_CPI;
			else if( strcmp(val, "CPL") == 0)		_recvLOG.ads_user_type = DEFINE_ADSTYPE_CPL;
			else if( strcmp(val, "CPC") == 0)		_recvLOG.ads_user_type = DEFINE_ADSTYPE_CPC;
			else if( strcmp(val, "CPS") == 0)		_recvLOG.ads_user_type = DEFINE_ADSTYPE_CPS;
			else if( strcmp(val, "CPA") == 0)		_recvLOG.ads_user_type = DEFINE_ADSTYPE_CPA;
				
		}
		
		else if(strcmp(name, "user_auto_share") == 0) {
			
			strcpy(&tmp, val);

			if( tmp == 'Y') 						_recvLOG.user_auto_share = ON;
			else if( tmp == 'N') 					_recvLOG.user_auto_share = OFF;
			else 									_recvLOG.user_auto_share = WAIT;
				
		}
		
        else if(strcmp(name, "uidx") == 0)			_recvLOG.uidx = atoi(val);
        else if(strcmp(name, "ads_info_idx") == 0)	_recvLOG.ads_info_idx = atoi(val);
        else if(strcmp(name, "fb_ads_idx") == 0)	_recvLOG.fb_ads_idx = atoi(val);
        else if(strcmp(name, "feed_idx") == 0)		_recvLOG.feed_idx = atoi(val);
        else if(strcmp(name, "able_cnt") == 0)		_recvLOG.able_cnt = atoi(val);
        else if(strcmp(name, "cost_adv") == 0)		_recvLOG.cost_adv = atoi(val);
        else if(strcmp(name, "cost_user") == 0)		_recvLOG.cost_user = atoi(val);
        else if(strcmp(name, "ads_cost") == 0)		_recvLOG.cost_adv = atoi(val);
        else if(strcmp(name, "ads_user_cost") == 0)	_recvLOG.cost_user = atoi(val);
        else if(strcmp(name, "gap_like") == 0)		_recvLOG.gap_like = atoi(val);

        else if(strcmp(name, "lncid") == 0 )		strcpy(_recvLOG.lncid,val);
        //else if(strcmp(name, "ip") == 0)			strcpy(_recvLOG.userip,val);
        	
        	
        else if(strcmp(name, "audience") == 0)		_recvLOG.audience = atoi(val);
        
        
   		m = strchr(m, ' ');
		m++;
	};

	return SUCCESS;

};









/**********************************************/
//  int DbMemoryLoad()
//   - 테이블에 있는 데이터를 메모리에 로드
/**********************************************/
int DbMemoryLoad()
{
	
	int i;
	
	char sql[1024];
	
	MYSQL		mysql; 
	MYSQL_RES*	res;
	MYSQL_ROW	row;
	
	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...DbMemoryLoad START", __FILE__, (long)getpid(), __LINE__, __func__);
	
	//////////////////////////
	// MySQl Init, Connect 
    mysql_init(&mysql);
    if ( mysql_real_connect(&mysql, DBIP, DBID, DBPW, DBNAME , DBPORT, (char *)NULL, 0) == NULL )
    {
        LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
        return FAIL;
    };
    
	
	//회원가입 카운트
    sprintf(sql, "SELECT (SELECT IFNULL(count(*),0) FROM user WHERE LEFT(create_time, 10) = '%s') AS cnt1, (SELECT IFNULL(count(*),0) FROM user WHERE LEFT(create_time, 10) <= '%s') AS cnt2,(SELECT distinct(count(uidx)) FROM log_login_user WHERE LEFT(create_time, 10) = '%s') AS cnt3", LogDate, LogDate, LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
    res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	_TOTAL.user_join = atoi(row[0]);
	_TOTAL.user_join_sum = atoi(row[1]);
	_TOTAL.user_login_unique = atoi(row[2]);
	mysql_free_result(res);	
		
	
	
	//친구수 카운트    
    sprintf(sql, "SELECT (SELECT IFNULL(SUM(fb_friends),0) FROM user_fb_data WHERE LEFT(create_time, 10) = '%s') AS cnt1, (SELECT IFNULL(SUM(fb_friends),0) FROM user_fb_data WHERE LEFT(create_time, 10) <= '%s') AS cnt2", LogDate, LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
    res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	_TOTAL.user_friends = atoi(row[0]);
	_TOTAL.user_friends_sum = atoi(row[1]);	
	mysql_free_result(res);
	
	
	
	
	//오토피드 카운트
    sprintf(sql, "SELECT (SELECT IFNULL(COUNT(*),0) FROM user WHERE user_auto_share = 'Y' AND LEFT(create_time, 10) = '%s') AS cnt1, (SELECT IFNULL(COUNT(*),0) FROM user WHERE user_auto_share = 'S' AND LEFT(create_time, 10) = '%s') AS cnt2, (SELECT IFNULL(COUNT(*),0) FROM user WHERE user_auto_share = 'N' AND LEFT(create_time, 10) = '%s') AS cnt3", LogDate, LogDate, LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
    res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	_TOTAL.user_auto_share_Y = atoi(row[0]);
	_TOTAL.user_auto_share_S = atoi(row[1]);
	_TOTAL.user_auto_share_N = atoi(row[2]);
	mysql_free_result(res);
	
	
	
	
	//캠페인 등록수
    sprintf(sql, "SELECT IFNULL(count(*),0) FROM fb_ads WHERE LEFT(create_time, 10) = '%s'", LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
    res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	_TOTAL.fb_ads_count = atoi(row[0]);
	mysql_free_result(res);
	
	
	/*
	// USER Nlog
	sprintf(sql, "SELECT uidx FROM user ORDER BY uidx ASC");
	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	res = mysql_store_result(&mysql);
	_UIDXcnt = (int)mysql_num_rows(res);
	_UIDX = (stNLOG_UIDX *)calloc(sizeof(stNLOG_UIDX), _UIDXcnt );
	for(i = 0 ; i < _UIDXcnt ; i++)
	{
		row = mysql_fetch_row(res);		
		_UIDX[i].uidx = atoi(row[0]);
	};
	mysql_free_result(res);
	*/
	
	
	//ADS_INFO Nlog
	sprintf(sql, "SELECT ads_info_idx FROM ads_info ORDER BY ads_info_idx ASC");
	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	res = mysql_store_result(&mysql);
	_ADSINFOcnt = (int)mysql_num_rows(res);
	_ADSINFO = (stNLOG_ADSINFO *)calloc(sizeof(stNLOG_ADSINFO), _ADSINFOcnt );
	for(i = 0 ; i < _ADSINFOcnt ; i++)
	{
		row = mysql_fetch_row(res);		
		_ADSINFO[i].ads_info_idx = atoi(row[0]);
	};
	mysql_free_result(res);	
	
	
	//_FB_ADS 캠페인별 Nlog
	sprintf(sql, "SELECT fb_ads_idx FROM fb_ads ORDER BY fb_ads_idx ASC");
	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	res = mysql_store_result(&mysql);
	_FB_ADScnt = (int)mysql_num_rows(res);
	_FB_ADS = (stNLOG_FB_ADS *)calloc(sizeof(stNLOG_FB_ADS), _FB_ADScnt );
	for(i = 0 ; i < _FB_ADScnt ; i++)
	{
		row = mysql_fetch_row(res);		
		_FB_ADS[i].fb_ads_idx = atoi(row[0]);
	};
	mysql_free_result(res);
	
	
	mysql_close(&mysql);
	
	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...DbMemoryLoad END", __FILE__, (long)getpid(), __LINE__, __func__);
	
	
	return SUCCESS;
	
}




/*********************************************/
// DB_Insert_Process
/*********************************************/
int DB_Insert_Process()
{
	int i, j;
	
	int clk, uclk, like;
	
	
	char sql[2048];	
	
	MYSQL		mysql; 
	MYSQL_RES*	res;
	MYSQL_ROW	row;
	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...DB_Insert_Process Start", __FILE__, (long)getpid(), __LINE__, __func__);
	
	
	// MySQl Init, Connect
    mysql_init(&mysql);
    if ( mysql_real_connect(&mysql, DBIP, DBID, DBPW, DBNAME, DBPORT, (char *)NULL, 0) == NULL )
    {
        LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
        return FAIL;
    };
    
    
    
	//일별 리포트 삭제 처리
    sprintf(sql, "DELETE FROM rep_day_total WHERE logdate = '%s'", LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	
		
    		
	sprintf(sql, "INSERT INTO rep_day_total (logdate, user_join, user_join_sum, user_friends, user_friends_sum, user_del, user_auto_share_Y, user_auto_share_S, user_auto_share_N, user_login_unique, user_login_web, user_login_aos, user_login_ios, fb_ads_count, fb_feed_share_auto, fb_feed_share_self, total_click, total_uclick, total_like_sum, total_like_count, total_like_cost_adv,  total_like_cost_user,  total_cpc_click,  total_cpc_cost_adv,  total_cpc_cost_user, total_ncpc_click,  total_ncpc_cost_adv,  total_ncpc_cost_user,  total_cpi_count,  total_cpi_cost_adv,  total_cpi_cost_user, total_cps_count,  total_cps_cost, total_cpa_count,  total_cpa_cost) VALUES ('%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)"
			,LogDate
			,_TOTAL.user_join
			,_TOTAL.user_join_sum
			,_TOTAL.user_friends
			,_TOTAL.user_friends_sum
			,_TOTAL.user_del
			,_TOTAL.user_auto_share_Y
			,_TOTAL.user_auto_share_S
			,_TOTAL.user_auto_share_N
			,_TOTAL.user_login_unique
			,_TOTAL.user_login_web
			,_TOTAL.user_login_aos
			,_TOTAL.user_login_ios
			,_TOTAL.fb_ads_count
			,_TOTAL.fb_feed_share_auto
			,_TOTAL.fb_feed_share_self
			,_TOTAL.click
			,_TOTAL.uclick
			,_TOTAL.total_like_sum
			,_TOTAL.total_like_count
			,_TOTAL.total_like_cost_adv
			,_TOTAL.total_like_cost_user
			,_TOTAL.total_cpc_click
			,_TOTAL.total_cpc_cost_adv
			,_TOTAL.total_cpc_cost_user
			,_TOTAL.total_ncpc_click
			,_TOTAL.total_ncpc_cost_adv
			,_TOTAL.total_ncpc_cost_user
			,_TOTAL.total_cpi_count
			,_TOTAL.total_cpi_cost_adv
			,_TOTAL.total_cpi_cost_user
			,_TOTAL.total_cps_count
			,_TOTAL.total_cps_cost
			,_TOTAL.total_cpa_count
			,_TOTAL.total_cpa_cost
			
	);
	
	//printf("%s\n", sql);

				
	if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	
	
	// fb_ads_info
	sprintf(sql, "DELETE FROM rep_day_ads_info WHERE logdate = '%s'", LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	for(i = 0 ; i < _ADSINFOcnt ; i++)
	{
		
		if(_ADSINFO[i].chk == 1) {
			
			sprintf(sql, "INSERT INTO rep_day_ads_info (logdate, ads_info_idx, total_click, total_uclick) VALUES ('%s', %d, %d, %d)"
				,LogDate
				,_ADSINFO[i].ads_info_idx
				,_ADSINFO[i].click
				,_ADSINFO[i].uclick
			);
			
			if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
			{
				LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
				return FAIL;
			};
		};
	}
	
	/*
	// fb_ads
	sprintf(sql, "DELETE FROM rep_day_fb_ads WHERE logdate = '%s'", LogDate);
    if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
		return FAIL;
	};
	*/
	
	for(i = 0 ; i < _FB_ADScnt ; i++)
	{
		
		if(_FB_ADS[i].chk == 1)
		{
			
			sprintf(sql, "SELECT COUNT(*) FROM rep_day_fb_ads WHERE fb_ads_idx = '%d' AND logdate = '%s'",  _FB_ADS[i].fb_ads_idx, LogDate);
			if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
			{
				LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
				return FAIL;
			};
		    res = mysql_store_result(&mysql);
			row = mysql_fetch_row(res);			
				
			//INSERT	
			if( atoi(row[0]) < 1) {
				
				sprintf(sql, "INSERT INTO rep_day_fb_ads (logdate, fb_ads_idx, fb_feed_share_auto, fb_feed_share_self, fb_feed_audience, total_click, total_uclick, total_like_sum, total_like_count, total_like_cost_adv, total_like_cost_user, total_cpc_click, total_cpc_cost_adv, total_cpc_cost_user, total_cpi_count, total_cpi_cost_adv, total_cpi_cost_user, total_cps_count, total_cps_cost, total_cpa_count, total_cpa_cost) VALUES ('%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,%d, %d)"
					,LogDate
					,_FB_ADS[i].fb_ads_idx
					,_FB_ADS[i].fb_feed_share_auto
					,_FB_ADS[i].fb_feed_share_self
					,_FB_ADS[i].fb_feed_audience
					,_FB_ADS[i].click
					,_FB_ADS[i].uclick
					,_FB_ADS[i].total_like_sum
					,_FB_ADS[i].total_like_count
					,_FB_ADS[i].total_like_cost_adv
					,_FB_ADS[i].total_like_cost_user
					,_FB_ADS[i].total_cpc_click
					,_FB_ADS[i].total_cpc_cost_adv
					,_FB_ADS[i].total_cpc_cost_user
					,_FB_ADS[i].total_cpi_count
					,_FB_ADS[i].total_cpi_cost_adv
					,_FB_ADS[i].total_cpi_cost_user
					,_FB_ADS[i].total_cps_count
					,_FB_ADS[i].total_cps_cost
					,_FB_ADS[i].total_cpa_count
					,_FB_ADS[i].total_cpa_cost
				);
				
				if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
				{
					LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
					return FAIL;
				};
				
			//UPDATE
			}else {
			
				sprintf(sql, "UPDATE rep_day_fb_ads SET fb_feed_share_auto = '%d',fb_feed_share_self = '%d',fb_feed_audience = '%d',total_click = '%d',total_uclick = '%d',total_like_sum = '%d',total_like_count = '%d',total_like_cost_adv = '%d',total_like_cost_user = '%d',total_cpc_click = '%d',total_cpc_cost_adv = '%d',total_cpc_cost_user = '%d',total_cpi_count = '%d',total_cpi_cost_adv = '%d',total_cpi_cost_user = '%d',total_cps_count = '%d',total_cps_cost = '%d',total_cpa_count = '%d',total_cpa_cost = '%d' WHERE logdate = '%s' AND fb_ads_idx = '%d'"
					,_FB_ADS[i].fb_feed_share_auto
					,_FB_ADS[i].fb_feed_share_self
					,_FB_ADS[i].fb_feed_audience
					,_FB_ADS[i].click
					,_FB_ADS[i].uclick
					,_FB_ADS[i].total_like_sum
					,_FB_ADS[i].total_like_count
					,_FB_ADS[i].total_like_cost_adv
					,_FB_ADS[i].total_like_cost_user
					,_FB_ADS[i].total_cpc_click
					,_FB_ADS[i].total_cpc_cost_adv
					,_FB_ADS[i].total_cpc_cost_user
					,_FB_ADS[i].total_cpi_count
					,_FB_ADS[i].total_cpi_cost_adv
					,_FB_ADS[i].total_cpi_cost_user
					,_FB_ADS[i].total_cps_count
					,_FB_ADS[i].total_cps_cost
					,_FB_ADS[i].total_cpa_count
					,_FB_ADS[i].total_cpa_cost
					,LogDate
					,_FB_ADS[i].fb_ads_idx
				);
				
				if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
				{
					LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
					return FAIL;
				};
				
			};
			
			/*
			sprintf(sql, "UPDATE fb_ads SET total_click = '%d', total_unique_click = '%d' WHERE fb_ads_idx = '%d'", _FB_ADS[i].click, _FB_ADS[i].uclick, _FB_ADS[i].fb_ads_idx);
			if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
			{
				LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
				return FAIL;
			};
			*/
			
		
			sprintf(sql, "SELECT SUM(total_click), SUM(total_uclick), SUM(total_like_sum) FROM rep_day_fb_ads WHERE fb_ads_idx = '%d'",  _FB_ADS[i].fb_ads_idx );
			if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
			{
				LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
				return FAIL;
			};
		    res = mysql_store_result(&mysql);
			row = mysql_fetch_row(res);			
			clk = atoi(row[0]);
			uclk = atoi(row[1]);
			like = atoi(row[2]);
			mysql_free_result(res);
			
			sprintf(sql, "UPDATE fb_ads SET total_click = '%d', total_unique_click = '%d', total_like_sum = '%d' WHERE fb_ads_idx = '%d'", clk, uclk, like, _FB_ADS[i].fb_ads_idx);
			if( mysql_real_query(&mysql,sql,strlen(sql)) != 0 )
			{
				LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...error(%s)", __FILE__, (long)getpid(), __LINE__, __func__, mysql_error(&mysql));
				return FAIL;
			};
						
		};
		
		
	}
	
	
	
    
    
	
	mysql_close(&mysql);
	
		
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...DB_Insert_Process End", __FILE__, (long)getpid(), __LINE__, __func__);
	
	
	return SUCCESS;
	
};


/*********************************************/
// stNLOG_ADSINFO_PROC(0, _ADSINFOcnt, _recvLOG.ads_info_idx, uchk);
/*********************************************/
int stNLOG_ADSINFO_PROC(int low, int high, uint ads_info_idx, uchar uchk)
{
	
	int i;
	
	if(low > high) return;
	
	i = (low + high) / 2;
	
	if( _ADSINFO[i].ads_info_idx == ads_info_idx )
	{	
		_ADSINFO[i].chk = 1;
		
		_ADSINFO[i].click++;
		if(uchk) _ADSINFO[i].uclick++;
			
		return;
	}	
	else if(_ADSINFO[i].ads_info_idx < ads_info_idx)
	{
		low = i + 1;
		stNLOG_ADSINFO_PROC(low, high, ads_info_idx, uchk);
	}
	else
	{
		high = i - 1;
		stNLOG_ADSINFO_PROC(low, high, ads_info_idx, uchk);
	};
}



/*********************************************/
int stNLOG_FB_ADS_PROC(int low, int high, uchar type, uchar type2, uchar type3, uint fb_ads_idx, uchar uchk, uint able_cnt, uint cost_adv, uint cost_user, uint audience)
{
	
	int i;
	
	if(low > high) return;
	
	i = (low + high) / 2;
	
	if( _FB_ADS[i].fb_ads_idx == fb_ads_idx )
	{	
		_FB_ADS[i].chk = 1;
		
		if(type == DEFINE_LOG_CLICK)
		{			
			
			_FB_ADS[i].click++;
			if(uchk) _FB_ADS[i].uclick++;
				
		}
		else if(type == DEFINE_LOG_NCLICK)
		{
			
			//총 좋아요
			if(type2 == DEFINE_TYPE_CPL) {
				_FB_ADS[i].total_like_sum += able_cnt;
			}			
			
		}
		else if(type == DEFINE_LOG_VALID)
		{			
			//공유 건수 분석
			if(type2 == DEFINE_TYPE_SHARE)
			{				
				if(type3 == DEFINE_TYPE3_SHARE_AUTO)		_FB_ADS[i].fb_feed_share_auto++;
				else if(type3 == DEFINE_TYPE3_SHARE_SELF)	_FB_ADS[i].fb_feed_share_self++;			
					
				_FB_ADS[i].fb_feed_audience += audience;
			}
			else if(type2 == DEFINE_TYPE_CPL)
			{
				_FB_ADS[i].total_like_count += able_cnt;
				_FB_ADS[i].total_like_cost_adv += cost_adv;
				_FB_ADS[i].total_like_cost_user += cost_user;
			}
			else if(type2 == DEFINE_TYPE_CPC)
			{
				_FB_ADS[i].total_cpc_click++;
				_FB_ADS[i].total_cpc_cost_adv += cost_adv;
				_FB_ADS[i].total_cpc_cost_user += cost_user;
			}
			else if(type2 == DEFINE_TYPE_CPA)
			{
				_FB_ADS[i].total_cpa_count++;
				_FB_ADS[i].total_cpa_cost += cost_user;
			}
			
		}
		else if(type == DEFINE_LOG_POSTBACK)
		{
			if(type2 == DEFINE_TYPE_INSTALL)
			{
				_FB_ADS[i].total_cpi_count++;
				_FB_ADS[i].total_cpi_cost_adv += cost_adv;
				_FB_ADS[i].total_cpi_cost_user += cost_user;
			}
		};
			
		return;
	}	
	else if(_FB_ADS[i].fb_ads_idx < fb_ads_idx)
	{
		low = i + 1;
		stNLOG_FB_ADS_PROC(low, high, type, type2, type3, fb_ads_idx, uchk, able_cnt, cost_adv, cost_user, audience);
	}
	else
	{
		high = i - 1;
		stNLOG_FB_ADS_PROC(low, high, type, type2, type3, fb_ads_idx, uchk, able_cnt, cost_adv, cost_user, audience);
	};
}


	
/*********************************************/
// NLOG_UIDX_PROC
/*********************************************/
/*
int NLOG_UIDX_PROC(int low, int high, uchar type, uint uidx, uint able_cnt, uint cost_adv, uint cost_user)
{	
	
	int i;
	
	if(low > high) return;
	
	i = (low + high) / 2;
	
	if( _FB_ADS[i].fb_ads_idx == fb_ads_idx )
	{		
		_FB_ADS[i].click++;		
		return;
	}	
	else if(_FB_ADS[i].fb_ads_idx < fb_ads_idx)
	{
		low = i + 1;
		NLOG_UIDX_PROC(low, high, fb_ads_idx);
	}
	else
	{
		high = i - 1;
		NLOG_UIDX_PROC(low, high, fb_ads_idx);
	};
}
*/

/*********************************************/
// CLICK_PROC
/*********************************************/
int CLICK_PROC(int low, int high, uint fb_ads_idx)
{	
	
	int i;
	
	if(low > high) return;
	
	i = (low + high) / 2;
	
	if( _FB_ADS[i].fb_ads_idx == fb_ads_idx )
	{		
		_FB_ADS[i].click++;		
		return;
	}	
	else if(_FB_ADS[i].fb_ads_idx < fb_ads_idx)
	{
		low = i + 1;
		CLICK_PROC(low, high, fb_ads_idx);
	}
	else
	{
		high = i - 1;
		CLICK_PROC(low, high, fb_ads_idx);
	};
}

void Memory_Free()
{
	/*
	if(_UIDX) {
		free(_UIDX);
		_UIDX = NULL;
	}
	*/
	
	if(_ADSINFO) {
		free(_ADSINFO);
		_ADSINFO = NULL;
	}
	
	if(_FB_ADS) {
		free(_FB_ADS);
		_FB_ADS = NULL;
	}
	
	
}


/**********************************************/
// LogFile_Process
/**********************************************/
void* LogFile_Process()
{
	char line[LINE_MAX];


	char tmp[100];
	char uniqueID[51];
	
	uchar uchk = 0;
	
	time_t	pt;
	struct	tm	lt;

	FILE *fp;
	char *p;

	//int d = 0, h = 0, m = 0, s = 0;

	time(&pt);
	localtime_r(&pt, &lt);

	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...LogFile Analysis START(%s)", __FILE__, (long)getpid(), __LINE__, __func__, LoadFileName);
	
	
	fp = fopen(LoadFileName, "r");

	
	hcreate(MEMORY_LIMIT_CNT);	
	
	
	if(fp)
	{
		
		while(fgets(line, LINE_MAX, fp))
		{
			
			_LINE_COUNT++;			
			
			if(line[0] != 't') continue;
			
			p = strstr(line, "referer=");
			if(p) *p = 0;
			
			
			//printf("%s\n", line);			
				

			LineParser(line);
			
			//printf("%s\n", _recvLOG.userip);
			

			//가입수는 DB에서 가져오는걸로
			if(_recvLOG.type == DEFINE_LOG_JOIN)
			{
				//_TOTAL.user_join++;
				continue;
			}
			
			//로그인수 타입별 카운트
			else if(_recvLOG.type == DEFINE_LOG_LOGIN)
			{
				if(_recvLOG.type2 == DEFINE_TYPE_LOGIN)
				{
					if(_recvLOG.type3 == DEFINE_TYPE3_LOGIN_WEB) 		_TOTAL.user_login_web++;
					else if(_recvLOG.type3 == DEFINE_TYPE3_LOGIN_AOS) 	_TOTAL.user_login_aos++;
					else if(_recvLOG.type3 == DEFINE_TYPE3_LOGIN_IOS)	_TOTAL.user_login_ios++;
				}
			}
			
			// 클릭
			else if(_recvLOG.type == DEFINE_LOG_CLICK)
			{
				    
				//총 클릭수 & 유니크 클릭수
				_TOTAL.click++;
				
				if(_recvLOG.lncid[0])
				{
					memset(uniqueID, 0x00, sizeof(uniqueID));
					sprintf(tmp, "T%s", _recvLOG.lncid);
					SHA1Encode(tmp, uniqueID);
					hash_item.key = uniqueID;
					if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
					{
						hash_item.key	= uniqueID;
						hash_item.data	= "1";
						hsearch(hash_item, ENTER);

						_TOTAL.uclick++;
					};
				};
				
				
				uchk = 0;
				if(_recvLOG.lncid[0])
				{
					memset(uniqueID, 0x00, sizeof(uniqueID));
					sprintf(tmp, "Z%d%s", _recvLOG.ads_info_idx, _recvLOG.lncid);
					SHA1Encode(tmp, uniqueID);
					hash_item.key = uniqueID;
					if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
					{
						hash_item.key	= uniqueID;
						hash_item.data	= "1";
						hsearch(hash_item, ENTER);
						
						uchk = 1;
					};
				};
				
				//피드별 클릭수 & 유니크 클릭수
				stNLOG_ADSINFO_PROC(0, _ADSINFOcnt, _recvLOG.ads_info_idx, uchk);
				
				
				uchk = 0;
				if(_recvLOG.lncid[0])
				{
					memset(uniqueID, 0x00, sizeof(uniqueID));
					sprintf(tmp, "X%d%s", _recvLOG.fb_ads_idx, _recvLOG.lncid);
					SHA1Encode(tmp, uniqueID);
					hash_item.key = uniqueID;
					if ( NULL == (hash_result = hsearch( hash_item, FIND)) )
					{
						hash_item.key	= uniqueID;
						hash_item.data	= "1";
						hsearch(hash_item, ENTER);

						uchk = 1;
					};
				};
				
				//캠페인별 클릭수 & 유니크 클릭수
				stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, _recvLOG.able_cnt, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
			
			}
			
			//비과금
			else if(_recvLOG.type == DEFINE_LOG_NCLICK)
			{
				//비과금 클릭형 분석
				if(_recvLOG.type2 == DEFINE_TYPE_CPC)
				{
					_TOTAL.total_ncpc_click++;
					_TOTAL.total_ncpc_cost_adv += _recvLOG.cost_adv;
					_TOTAL.total_ncpc_cost_user += _recvLOG.cost_user;
				}					
				//총 좋아요
				else if(_recvLOG.type2 == DEFINE_TYPE_CPL)
				{
					
					_TOTAL.total_like_sum += _recvLOG.gap_like;
					
					//캠페인별 총 좋아요
					stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, _recvLOG.gap_like, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
				}				
			}			
			//공유건수 로그
			else if(_recvLOG.type == DEFINE_LOG_VALID)
			{
				//공유 건수 분석
				if(_recvLOG.type2 == DEFINE_TYPE_SHARE)
				{
					if(_recvLOG.type3 == DEFINE_TYPE3_SHARE_AUTO)		_TOTAL.fb_feed_share_auto++;
					else if(_recvLOG.type3 == DEFINE_TYPE3_SHARE_SELF)	_TOTAL.fb_feed_share_self++;
						
					//캠페인별 클릭수 & 유니크 클릭수
					stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, _recvLOG.able_cnt, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
					
				}
				
				//CPA 분석
				else if(_recvLOG.type2 == DEFINE_TYPE_CPA)
				{
    
					_TOTAL.total_cpa_count += _recvLOG.able_cnt;
					_TOTAL.total_cpa_cost += _recvLOG.cost_user;
					
					//캠페인별					
					stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, _recvLOG.able_cnt, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
									
				}				
				
				//좋아요 분석
				else if(_recvLOG.type2 == DEFINE_TYPE_CPL)
				{
					_TOTAL.total_like_count += _recvLOG.able_cnt;
					_TOTAL.total_like_cost_adv += _recvLOG.cost_adv;
					_TOTAL.total_like_cost_user += _recvLOG.cost_user;	
					
					//캠페인별					
					stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, _recvLOG.able_cnt, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
					
					
					//공유별
					
					
				}
				
				//클릭형 분석
				else if(_recvLOG.type2 == DEFINE_TYPE_CPC)
				{
					_TOTAL.total_cpc_click++;
					_TOTAL.total_cpc_cost_adv += _recvLOG.cost_adv;
					_TOTAL.total_cpc_cost_user += _recvLOG.cost_user;
					
					//캠페인별					
					//캠페인별					
					stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, _recvLOG.able_cnt, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
					
					
					//공유별
					
				}
				
				//구매형 분석
				else if(_recvLOG.type2 == DEFINE_TYPE_CPS)
				{
					_TOTAL.total_cps_count++;
					_TOTAL.total_cps_cost += _recvLOG.cost_adv;
					
				}
				
			}
			
			//포스트백
			else if(_recvLOG.type == DEFINE_LOG_POSTBACK)
			{
				
				//페이스북 권한삭제
				if(_recvLOG.type2 == DEFINE_TYPE_CANCEL)
				{
					_TOTAL.user_del++;
				}
				
				//전체 설치건
				else if(_recvLOG.type2 == DEFINE_TYPE_INSTALL)
				{
					_TOTAL.total_cpi_count++;
					_TOTAL.total_cpi_cost_adv += _recvLOG.cost_adv;
					_TOTAL.total_cpi_cost_user += _recvLOG.cost_user;
					
					//캠페인별 클릭수 & 유니크 클릭수
					stNLOG_FB_ADS_PROC(0, _FB_ADScnt, _recvLOG.type, _recvLOG.type2, _recvLOG.type3, _recvLOG.fb_ads_idx, uchk, 1, _recvLOG.cost_adv, _recvLOG.cost_user, _recvLOG.audience);
					
					
				}
			}
			
		};
		
		fclose(fp);
	};
	
	
	hdestroy();
	
	
	
	DB_Insert_Process();
	
	
	Memory_Free();
	
	
	PROCESS = true;
	
};



/* ================================================================================================
 *  main
 * ============================================================================================= */
int main(int argc, char *argv[])
{
   
	int pthread_result;
	pthread_t thread1, thread2;
	
	time_t	pt;
	struct	tm	lt;
	
	char tmpdate[20];
	
	
	if(argc == 2)
	{	
		MANUAL = true;
		
		/////////////////////////
		//	Log 파일 수동 분석
		if ( strcmp(argv[1], "restore") == 0 )
		{
			puts("Enter your restore date(ex. 2016-01-01)");
			scanf("%s", LogDate);
				
		}else {
			
			fprintf(stderr, "Unknow Code.......Usage:[./nlogday restore]\n");
			exit(EXIT_FAILURE);
		}
	}
	else if(argc == 3)
	{
		MANUAL = true;
		
		/////////////////////////
		//	Log 파일 수동 분석
		if ( strcmp(argv[1], "restore") == 0 )
		{
			if(argv[2]) {
				sprintf(LogDate, "%s", argv[2]);
			}else {
				fprintf(stderr, "Unknow Code.......Usage:[./nlogday restore 2016-01-01]\n");
				exit(EXIT_FAILURE);
			};
		}else {			
			fprintf(stderr, "Unknow Code.......Usage:[./nlogday restore 2016-01-01]\n");
			exit(EXIT_FAILURE);
		};
	}
	else
	{
		MANUAL = false;
		
		time(&pt);
		pt -= (60*15);
		localtime_r(&pt, &lt);
		sprintf(LogDate, "%04d-%02d-%02d",  lt.tm_year+1900, lt.tm_mon+1, lt.tm_mday);
	};
	
	// 로그 타입(수동분석 => 출력)	
	if(MANUAL) LOGSHOW = SHOW;
	else  LOGSHOW = DEFAULT;
	
	LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...ENGINE START", __FILE__, (long)getpid(), __LINE__, __func__);	
	
	
	sprintf(tmpdate,"%s", LogDate);	
	replace(tmpdate, "-", "");
	
	
	//로그 파일
	memset(LoadFileName, 0x00, sizeof(LoadFileName));
	sprintf(LoadFileName, "%s/%s.log", DAEMON_DATA, tmpdate);
	
	
	
	memset(&_TOTAL, 0, sizeof(stTOTAL));
    
    
    
	if( bIs_there_file(LoadFileName) < 0) 
	{	
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...File Not Found(%s)", __FILE__, (long)getpid(), __LINE__, __func__, LoadFileName);
		exit(FAIL);
		
	};
	
    
    
    //////////////////////////////////////////////
	//											//
	//		DB메모리 로드 				
	//											//
	//////////////////////////////////////////////
  	if(DbMemoryLoad() < 0) 
	{
		LogPrint(FAIL, DAEMON_ERROR_FILE, "%s[%ld]{%d} %s...DbMemoryLoad error", __FILE__, (long)getpid(), __LINE__, __func__);
  		exit(EXIT_FAILURE);
  	};
  	
  	
  	//////////////////////////////////////////////
	//											//
	//		로그 파일 파싱 및 분석				//
	//											//
	//////////////////////////////////////////////	
	if((pthread_result=pthread_create( &thread1, NULL, ProcessBAR, NULL)))
	{
		printf("%s[%ld]{%d} %s...ProcessBAR Thread creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
		
	}
	
	
	if((pthread_result=pthread_create( &thread2, NULL, LogFile_Process, NULL)))
	{
		printf("%s[%ld]{%d} %s...LogFile_Process Thread creation failed", __FILE__, (long)getpid(), __LINE__, __func__);
    }

		
	//Engine Thread End
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
    
    
    pthread_detach(thread1);
    pthread_detach(thread2);
    
			
  	
    LogPrint(LOGSHOW, __FILE__, "%s[%ld]{%d} %s...ENGINE END", __FILE__, (long)getpid(), __LINE__, __func__);
    
    
    
    return 0;
    
};


