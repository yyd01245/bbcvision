/*
	author yyd
		
*/

#ifndef __RECVQUEUE_H_
#define __RECVQUEUE_H_

//#include <pthread.h>
//#include "..\socketlayer\include\ip4socket.h"
#include <stdio.h>
//#include <sys/types.h>
#include <stdlib.h>
//#include <string.h>
#include <Windows.h>
#include <map>
#include "Common.h"
#include "TSStreamInfo.h"

//#include <netinet/in.h>
//#include <sys/socket.h>
//#include <errno.h>
//#include <arpa/inet.h>
//#include <pthread.h>
//#include <semaphore.h>
/*
void pthread_mutex_lock(void* param)
{
	//CRITICAL_SECTION *csParam = (CRITICAL_SECTION *)param;
	EnterCriticalSection((CRITICAL_SECTION *)param);
}

void pthread_mutex_unlock(void* param)
{
	LeaveCriticalSection((CRITICAL_SECTION *)param);
}

void pthread_mutex_destroy(void *param)
{

	//Destroy
}

void pthread_mutex_init(void* param,void *p)
{
	InitializeCriticalSection((CRITICAL_SECTION *)param);
}
*/



class  NewQueue
{
public:
	CRITICAL_SECTION locker;
	//pthread_cond_t cond;
//	sem_t m_sem_send;
	uint8_t* buf;
	int bufsize;
	int write_ptr;
	int read_ptr;
	int m_iport;
	int m_iSendPort;
	char m_cdstIP[256];
	bool m_boverlay;
	bool m_hsIDRFrame;
	bool m_bIsOverlaying;
	bool m_bInitDecoder;
	bool m_bDelayFrame;

		unsigned udp_recv_thread;
	bool m_bstop;

	FILE *m_logfp;
	FILE *m_Mediafp;

	NewQueue(int iport=12000);
	~NewQueue();

	static unsigned int _stdcall udp_ts_recv(void* param);
	void init_queue( int size,int iport,const char* dstip,short isendPort,FILE* fp=NULL,FILE* fpInfo=NULL,bool bNeedControlPlay=false);
	void free_queue();
	void put_queue( uint8_t* buf, int size);
	int get_queue(uint8_t* buf, int size);
	bool set_tsDecoder_stat(bool bstat);
	void clean_RecvQue();
	bool dumxer(unsigned char* buff,int ilen,int *iHandleLen,int iflag=7);
	void filterNullPacket(char* buff,int ilen);
	bool ParseMediaInfo(uint8_t *buff,int ilen);

	void Adjust_PMT_table(TS_PMT* packet ,unsigned char *buffer);
	int Adjust_PAT_table(TS_PAT* packet ,unsigned char *buffer);
	void Adjust_TS_packet_header(TS_packet_Header* pHeader,unsigned char *buffer);

	bool Adjust_PES_Pakcet(unsigned char *buffer,int ilen);
	int ParseStreamInfo(uint8_t *buff,int ilen);
	
	uint64_t Parse_PTS(unsigned char *pBuf);

	bool GetVideoESInfo(unsigned char *pBuf,int itempLen);

	bool ParseH264ES(unsigned char* pBuf,int itemplen);

	bool Find_Stream_IFrame(unsigned char *buffer,int ilen);

	int FilterRTPData(char* buff,int ilen);

	//设置码率周期
	bool Set_tsRate_period(int iperiod);

	//获取到码率
	bool Get_tsRate(int* iRate);

	bool Get_tsIFrame_size(int* iSize);

	bool m_bNeedControlPlay; //识别是否rtp数据

	TSstreamInfo m_tsStreamparse;

	HANDLE ThreadHandle;

	int m_iPMTPID;
	int m_iVideoPID;
	int m_iAudioPID;
	int m_iServerPID;
	int m_iPCRPID;

	MapPIDStreamType m_mapStreamPID;

	bool m_bHasPTS ;
	bool m_bHasDTS ;
	uint64_t m_llPCR;
	uint64_t m_llDts;
	uint64_t m_llPts;
	uint64_t m_llLastPts; //用于计算帧率

	double m_iFrameRate;

	int m_iFramTotal;

	int m_iGopSize;

	int m_iperiod;//码率周期
	int m_iRate;//kb
	long long m_ltotalByte;
};








#endif
