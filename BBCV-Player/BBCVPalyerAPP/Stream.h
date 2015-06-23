#ifndef _STREAM_H_
#define _STREAM_H_

/*
	json格式报文组装 发送
	clientsocket 由外部传
*/

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include <iostream>
#include "cJSON.h"
#include <map>


class Stream
{
public:
	Stream();
	virtual ~Stream();
/*
	virtual bool CleanStream(char *strSeesionId,char *strSID,char* strReSID,
					char *strAuthiName,char *strAuthcode,char *strSerialno,char *strMsg);
	virtual  bool CleanTask(){};

	virtual  bool ConnectServer();
	virtual  bool StartOneStream();
	virtual bool FreeOneStream();
	virtual bool GetTaskStatus();
*/
	int Recv_str(char *pRecvBuff,int *pbufflen);
	virtual  int ConnectServer(const char* phostIp,int iPort);
	virtual int  Send_str(char* pUrl);


	virtual bool Send_Jsoon_str(FILE *fp);

	int Recv_str_udp(char *pRecvBuff,int *pbufflen);
	virtual  int ConnectServer_udp(const char* phostIp,int iPort);
	virtual int  Send_str_udp(char* pUrl,int iLen);
	virtual bool Send_Jsoon_str_udp();

	virtual bool Parse_Json_str(char *strJson);

	virtual bool Requst_Json_str(int iType,const char* strRequstType,const char* strsecRequstContxt);

	//dstd::map<int,

	//输出端口ip
	char m_strdstIP[128];
	int m_idstport;
	int m_clientSocket;
	char m_Url[512];
	

	//报文回复
	cJSON *pRet_root;

};

#endif

