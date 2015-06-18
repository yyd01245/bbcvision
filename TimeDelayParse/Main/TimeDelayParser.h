#ifndef TIMEDELAYPARSE_H_
#define TIMEDELAYPARSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "TSStreamInfo.h"
#include "Stream.h"


const int iDataBuffLen = 10*1024*1024;

class TimeDelayParser
{
public:
	TimeDelayParser();
	~TimeDelayParser();

	int init(const char * ConfigFile,char* strRSMIP=NULL,char* iRSMPort=NULL,char* weburl=NULL);

	void get_time(char*buff,int siz);
	static void *Parse_recv_thread(void *arg);

	static void *TS_Recv_Thread(void *arg);

	int WriteTsFile(unsigned char* cData,int ilen);

	int ParseTSData(unsigned  char* cData,int ilen);
	
	bool GetBuff(char* cData,int *ilen);

	bool PutInBuff(char* cData,int ilen);

	int Createsocket();
	//vector 

	pthread_mutex_t m_mutexlocker;

	pthread_t m_iRcvThreadID;
	int m_iCheckflag;

	FILE *m_fpts;

private:	


	char m_SW_ip[256];

	int m_iSendRate;
	int m_iRecvPort;
	int m_iSendPort;

	int m_BindSocket;

	int m_iBeginPort;

	bool m_bSwitchFlag;
	

	int buffer_max_size;

	char *buffer;

	int read_index;
	int write_index;

	pthread_t read_thread_id;
	pthread_t write_thread_id;
	pthread_mutex_t m_mutex;	

	unsigned long real_send_bytes;
	bool m_bNeedFindIFrame;
	TSstreamInfo m_tsStreamPrase;

};


#endif
