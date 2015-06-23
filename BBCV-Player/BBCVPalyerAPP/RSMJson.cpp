#include "RSMJson.h"
#include <time.h>

RSMJsonObject::RSMJsonObject()
{
	m_bISLoging = false;
	m_bInit = false;

	char lbuf[32];

	struct tm tmnow;
	time_t now;
	now = time(NULL);
	tmnow = *(localtime(&now));	
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	sprintf(lbuf,"login%02d-%02d.log",systm.wHour,systm.wMinute);
	m_fp = fopen(lbuf,"w");
}

RSMJsonObject::~RSMJsonObject()
{


}
int RSMJsonObject::init(char *rsmIP,int irsmPort)
{
	if(m_bISLoging)
		return -1;

	memset(m_strRSMIP,0,sizeof(m_strRSMIP));
	strcpy(m_strRSMIP,rsmIP);
	m_iRSMPort = irsmPort;
	return 0;
}

unsigned int _stdcall RSMJsonObject::login_rsm_threadfun(void* param)
{
	RSMJsonObject* this0 = (RSMJsonObject*)param;

	int iNumber = this0->m_iLoginNumber;
	this0->m_bISLoging = true;
	int tsPort = atoi(this0->m_strDstPort);


	for(int i=0;i<iNumber;++i)
	{

		char txt[25] ={0};
		sprintf(txt,"%d",tsPort+i*2);
		int ret = this0->logintoRSM(this0->m_strUrl,this0->m_strDstIP,txt);


		if(ret < 0)
		{
			//记录登陆失败
		//	--i;
		}
		Sleep(1000*3);
	}

	this0->m_bISLoging = false;
	return 0;
}

int RSMJsonObject::login(char* weburl,char* tsIP,char* tsPort,int iNumber)
{

	strcpy(m_strDstIP,tsIP);
	strcpy(m_strDstPort,tsPort);
	strcpy(m_strUrl,weburl);
	m_iLoginNumber = iNumber;
	HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, login_rsm_threadfun, (LPVOID)this, 0, &m_login_thread);
	CloseHandle (ThreadHandle);


		
	return 0;
}


void RSMJsonObject::get_time(char*buff,int siz)
{
	char lbuf[32];

	struct tm tmnow;
	time_t now;
	now = time(NULL);
	tmnow = *(localtime(&now));	
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	strftime(lbuf,31,"%Y/%m/%d",&tmnow);
	sprintf(buff,"%s:%d:%d:%d:%03ld",lbuf,systm.wHour,systm.wMinute,
		systm.wSecond,systm.wMilliseconds);

}

int RSMJsonObject::logintoRSM(char* weburl,char* tsIP,char* tsPort)
{
	int iserverport = m_iRSMPort;
	Stream ptmpRequest;
	int ret = ptmpRequest.ConnectServer(m_strRSMIP,iserverport);
	if(ret < 0)
		return -1;
	//cJSON *pRet_root;
	ptmpRequest.pRet_root = cJSON_CreateObject();
	ptmpRequest.Requst_Json_str(2,"cmd","vnclogin");

	char txt[128] ={0};
	srand(time(NULL));
	sprintf(txt,"dfdafd-%d",rand());
	ptmpRequest.Requst_Json_str(2,"resid",txt);

	ptmpRequest.Requst_Json_str(2,"iip",tsIP);
	ptmpRequest.Requst_Json_str(2,"iport",tsPort);
	ptmpRequest.Requst_Json_str(2,"rate","3072");
	ptmpRequest.Requst_Json_str(2,"url",weburl);
	ptmpRequest.Requst_Json_str(2,"serialno",txt);

	if(m_fp)
	{
		char txt[128]={0};
		char txtinfo[48] ={0};
		get_time(txtinfo,48);

		sprintf(txt,"\n %s send :  ",txtinfo);
		fwrite(txt,1,strlen(txt),m_fp);
		fflush(m_fp);
	}
	ret = ptmpRequest.Send_Jsoon_str(m_fp);


	//接收回复报文
	char strRecvData[1024] ={0};
	int irecvlen = sizeof(strRecvData);
	ret = ptmpRequest.Recv_str(strRecvData,&irecvlen);

	if(m_fp)
	{
		char txt[128]={0};
		char txtinfo[48] ={0};
		get_time(txtinfo,48);

		sprintf(txt,"\n %s recv :  ",txtinfo);
		fwrite(txt,1,strlen(txt),m_fp);
		fwrite(strRecvData,1,irecvlen,m_fp);
		fflush(m_fp);
	}
	if(ret == -2)
	{
		return ret;//没有收到对方报文，重发一次。
	}

	ret = ptmpRequest.Parse_Json_str(strRecvData);
	if(m_fp)
	{
		//
		char txttmp[1024] = {0};
		char txtinfo[48] ={0};
		get_time(txtinfo,48);
		sprintf(txttmp,"\n %s login return %s  \n",txtinfo,ret ?"success ":"failed ");
		fwrite(txttmp,1,strlen(txttmp),m_fp);
		fflush(m_fp);
	}
	return ret;
}

int RSMJsonObject::logouttoRSM()
{

	return 0;
}
