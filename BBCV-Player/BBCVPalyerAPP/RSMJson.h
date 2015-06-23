#ifndef __RSMJSON_H_
#define __RSMJSON_H_
#include "Stream.h"
#include <process.h>
#include <time.h>

typedef struct {
	unsigned int dev_type;
} BlcYunKeyMsgHead;

typedef struct {
	unsigned int sequence_num;
	unsigned int key_value; 
	unsigned int key_status; 

} BlcYunKeyIrrMsgBody;

typedef struct {
	BlcYunKeyMsgHead head;
	BlcYunKeyIrrMsgBody body;
} BlcYunKeyIrrMsg;

class RSMJsonObject
{
public:
	RSMJsonObject();
	~RSMJsonObject();
	int login(char* weburl,char* tsIP,char* tsPort,int iNumber);

	static unsigned int _stdcall login_rsm_threadfun(void* param);

	int logintoRSM(char* weburl,char* tsIP,char* tsPort);
	int logouttoRSM();
	int init(char *rsmIP,int irsmPort);

	void get_time(char*buff,int siz);
private:
	
	char m_strRSMIP[256];
	int  m_iRSMPort;
	char m_strDstIP[256];
	char m_strDstPort[256]; //begin port;
	char m_strUrl[4096];
	int m_iLoginNumber;

	bool m_bISLoging;//ÕýÔÚµÇÂ½
	bool m_bInit;
	unsigned m_login_thread;
	FILE* m_fp;

};


#endif