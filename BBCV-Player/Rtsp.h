#ifndef _MYRTSP_H_
#define _MYRTSP_H_

#include"stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include <Windows.h>
#include <process.h>


class MyRTSPClient
{
public:
	MyRTSPClient();
	~MyRTSPClient();

	int RtspInit(char* strRtspUrl);

	int SendOptions(char *strOption,int ilen);

	static unsigned int _stdcall RecvRtsp_threadFun(void *param);

private:
	int SendRequest(char *buff,int ilen);

	int m_iRtspServerPort;
	char m_strRtspServerAddr[128];

};

typedef unsigned int uint32_t;


#define MAX_FIELDS 1024

typedef struct rtsp_s {

	int           s;

	char         *host;
	int           port;
	char         *path;
	char         *mrl;
	char         *user_agent;

	char         *server;
	unsigned int  server_state;
	uint32_t      server_caps;

	unsigned int  cseq;
	char         *session;

	char        *answers[MAX_FIELDS];   /* data of last message */
	char        *scheduled[MAX_FIELDS]; /* will be sent with next message */
}rtsp_t;


