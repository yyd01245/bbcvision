#include "Stream.h"


#pragma comment(lib,"ws2_32.lib")
Stream::Stream()
{

	// 加载socket动态链接库(dll)  
	WORD wVersionRequested;  
	WSADATA wsaData;    // 这结构是用于接收Wjndows Socket的结构信息的  
	int err;  

	wVersionRequested = MAKEWORD( 1, 1 );   // 请求1.1版本的WinSock库  

	err = WSAStartup( wVersionRequested, &wsaData );  
	if ( err != 0 ) {  
		return ;          // 返回值为零的时候是表示成功申请WSAStartup  
	}  

	if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 ) {  
		// 检查这个低字节是不是1，高字节是不是1以确定是否我们所请求的1.1版本  
		// 否则的话，调用WSACleanup()清除信息，结束函数  
		WSACleanup( );  
		return ;   
	}  

	pRet_root = NULL;
	m_clientSocket = -1;
	m_idstport = 0;
	memset(m_strdstIP,0,sizeof(m_strdstIP));
	memset(m_Url,0,sizeof(m_Url));
	
}
Stream::~Stream()
{
	if(m_clientSocket != -1)
		closesocket(m_clientSocket);
	
	m_clientSocket = -1;
}


int  Stream::Send_str_udp(char* pUrl,int iLen)
{
	char* cJsonBuff = pUrl;
	printf("-----%s \n",cJsonBuff);
	fflush(stdout);

	sockaddr_in sss_addr;

	sss_addr.sin_family = AF_INET ;

	sss_addr.sin_addr.s_addr = inet_addr(m_strdstIP) ;
	sss_addr.sin_port=htons(m_idstport);

	int len = sizeof(sss_addr);
	if(m_clientSocket != -1)
	{
		int iret = sendto(m_clientSocket, pUrl, iLen, 0,(struct sockaddr*)&sss_addr,len);
		if(iret < 0)
		{
			printf("error send %s \n",cJsonBuff);
			return -1;
		}
		printf("---send len=%d\n",strlen(cJsonBuff));
		printf("---send over\n");
		fflush(stdout);
	}

	return 0;

}

int Stream::Recv_str_udp(char *pRecvBuff,int *pbufflen)
{
	if(pRecvBuff == NULL)
		return -1;
	char alivetick[1024]={0};
	int iData=0;
	char strdata[32]={0};
	sockaddr_in sss_addr;

	int len = sizeof(struct sockaddr);

	memset(alivetick,0,sizeof(alivetick));
	int length = 0;
	int i_rc = 0, i_count = 0;
	int iRecvLen = 0;
	do
	{
		i_rc = recvfrom(m_clientSocket, alivetick + i_count, 2000 - i_count,0, (struct sockaddr*)&sss_addr,&len);
		if (i_rc <= 0)break;//异常关闭
		i_count += i_rc;
	} while (strstr(alivetick, "XXEE") == NULL);
	iRecvLen = i_count;
	if (iRecvLen <= 0){
		closesocket(m_clientSocket);
		m_clientSocket = -1;
		printf("----recv error \n");
		return -1;
	}
	printf("recv:%s \n",alivetick);
	int bufflen = *pbufflen;
	if(bufflen < iRecvLen)
	{	
		return -1;
	}
	memcpy(pRecvBuff,alivetick,iRecvLen);
	*pbufflen = iRecvLen;

	return 0;
}


bool Stream::Send_Jsoon_str_udp()
{


	char cJsonBuff[1024 * 2];
	char * m_tmp;
	m_tmp = cJSON_Print(pRet_root);
	memset(cJsonBuff, 0, sizeof(cJsonBuff));
	sprintf(cJsonBuff, "%sXXEE", m_tmp);
	free(m_tmp);
	printf("-----%s \n",cJsonBuff);
	fflush(stdout);
	cJSON_Delete(pRet_root);
	pRet_root = NULL;

	sockaddr_in sss_addr;

	sss_addr.sin_family = AF_INET ;

	sss_addr.sin_addr.s_addr = inet_addr(m_strdstIP) ;
	sss_addr.sin_port=htons(m_idstport);

	if(m_clientSocket != -1)
	{
		sendto(m_clientSocket, cJsonBuff, strlen(cJsonBuff), 0,(struct sockaddr*)&sss_addr,sizeof(struct sockaddr));
		printf("---send len=%d\n",strlen(cJsonBuff));
		printf("---send over\n");
		fflush(stdout);

	}
	return true;
}


int  Stream::Send_str(char* pUrl)
{
	char* cJsonBuff = pUrl;
	printf("-----%s \n",cJsonBuff);
	fflush(stdout);


	if(m_clientSocket != -1)
	{
		int iret = send(m_clientSocket, cJsonBuff, strlen(cJsonBuff), 0);
		if(iret < 0)
		{
			printf("error send %s \n",cJsonBuff);
			return -1;
		}
		printf("---send len=%d\n",strlen(cJsonBuff));
		printf("---send over\n");
		fflush(stdout);
	}
	
	return 0;
	
}

int Stream::Recv_str(char *pRecvBuff,int *pbufflen)
{
	if(pRecvBuff == NULL)
		return -1;
	char alivetick[1024]={0};
	int iData=0;
	char strdata[32]={0};
	sockaddr_in sss_addr;

	int len = sizeof(struct sockaddr);

	memset(alivetick,0,sizeof(alivetick));
	int length = 0;
	int i_rc = 0, i_count = 0;
	int iRecvLen = 0;
	int iloop = 25;
	do
	{
		i_rc = recv(m_clientSocket, alivetick + i_count, 2000 - i_count,0);
		if (i_rc <= 0)
		{
			int ierror = WSAGetLastError();
			if(ierror == WSAEWOULDBLOCK)  //Resource temporarily unavailable continue;
			{ 
				--iloop;
				if(iloop < 0)
					break;
				else
				{
					Sleep(1);
					continue;
				}
			}
			else
				break;//异常关闭

		}
		i_count += i_rc;
	} while (strstr(alivetick, "XXEE") == NULL);
	iRecvLen = i_count;
	if (iRecvLen <= 0){
		int ierror = WSAGetLastError();
		closesocket(m_clientSocket);
		m_clientSocket = -1;
		printf("----recv error \n");
		return -2;
	}
	printf("recv:%s \n",alivetick);
	int bufflen = *pbufflen;
	if(bufflen < iRecvLen)
	{	
		return -1;
	}
	memcpy(pRecvBuff,alivetick,iRecvLen);
	*pbufflen = iRecvLen;

	return 0;
}


bool Stream::Send_Jsoon_str(FILE *fp)
{


	char cJsonBuff[1024 * 2];
	char * m_tmp;
	m_tmp = cJSON_Print(pRet_root);
	memset(cJsonBuff, 0, sizeof(cJsonBuff));
	sprintf(cJsonBuff, "%sXXEE", m_tmp);
	free(m_tmp);
	printf("-----%s \n",cJsonBuff);
	fflush(stdout);
	cJSON_Delete(pRet_root);
	pRet_root = NULL;

	sockaddr_in sss_addr;

	sss_addr.sin_family = AF_INET ;

	sss_addr.sin_addr.s_addr = inet_addr(m_strdstIP) ;
	sss_addr.sin_port=htons(m_idstport);
	
	if(m_clientSocket != -1)
	{
		if(fp)
		{
			fwrite(cJsonBuff,1,strlen(cJsonBuff),fp);
			fflush(fp);
		}
		int ret = send(m_clientSocket, cJsonBuff, strlen(cJsonBuff), 0);
		if(ret < 0)
		{
			int ierror = WSAGetLastError();
			return false;
		}
//		printf("---send len=%d\n",strlen(cJsonBuff));
//		printf("---send over\n");
//		fflush(stdout);


	}
	return true;
}

bool Stream::Parse_Json_str(char *cBuffRecv)
{
	cJSON *pItem = NULL;
	cJSON *pcmd = NULL;
	cJSON *pSerialNo = NULL;
	cJSON *pRet_root = NULL;
	cJSON* pRoot = cJSON_Parse(cBuffRecv);
	if (pRoot)
	{
		pSerialNo = cJSON_GetObjectItem(pRoot, "serialno");
		pcmd = cJSON_GetObjectItem(pRoot, "cmd");
		if (pcmd)
		{
			
			//判断请求类?
#if 1
			if (strcmp(pcmd->valuestring, "vnclogin") == 0)
			{
				//获取返回码
				cJSON* pRetCode = cJSON_GetObjectItem(pRoot, "retcode");
				char strretcode[128]={0};
				memcpy(strretcode,pRetCode->valuestring,strlen(pRetCode->valuestring)+1);
				if(atoi(strretcode) >= 0)
				{
					//success
					return true;
				}
				else
				{
					//failed
					return false;
				}
			
			}
			else if (strcmp(pcmd->valuestring, "vnclogout") == 0)
			{
				//获取返回码
				cJSON* pRetCode = cJSON_GetObjectItem(pRoot, "retcode");
				char strretcode[128]={0};
				memcpy(strretcode,pRetCode->valuestring,strlen(pRetCode->valuestring)+1);
				if(atoi(strretcode) >= 0)
				{
					//success
					return true;
				}
				else
				{
					//failed
					return false;
				}
			}
			//状态监测暂时无
#endif				
		}
	}
	
	return false;
}


bool Stream::Requst_Json_str(int iType,const char* strRequstType,const char* strsecRequstContxt)
{
	if(NULL == pRet_root)
	{
		printf("Json error no create \n");
		fflush(stdout);
		return false;
	}
	switch(iType)
	{
		case 1:
		{
			//int 
			
			cJSON_AddNumberToObject(pRet_root, strRequstType, atoi(strsecRequstContxt));
		}
		break;
		case 2:
		{
			//string
			cJSON_AddStringToObject(pRet_root, strRequstType, strsecRequstContxt);
		}
		break;
		default:
		{
			//string
			cJSON_AddStringToObject(pRet_root, strRequstType, strsecRequstContxt);
		}
		break;
	}

}

/*
int Stream::ConnectServer(const char* phostIp,int iPort)
{
	sockaddr_in s_addr;
	return true;
}
*/

int Stream::ConnectServer(const char* phostIp,int iPort)
{
	sockaddr_in sss_addr;
		int sockid;
		
		sockid=socket(AF_INET, SOCK_STREAM, 0);
			
		sss_addr.sin_family = AF_INET ;
	
		sss_addr.sin_addr.s_addr = inet_addr(phostIp) ;
		sss_addr.sin_port=htons((unsigned short)iPort);
		strcpy(m_strdstIP,phostIp);
		m_idstport  = iPort;

		int iret = -1;
		iret = connect(sockid,(struct sockaddr *)&sss_addr,(int)(sizeof(sss_addr)));
		if(iret == -1)
		{
			return -1;
		}

		 unsigned long ul = 1;
/*		 ioctlsocket(sockid, FIONBIO, &ul); //设置为非阻塞模式
		 int len = sizeof(int);
		 int nRecvBuf = 3000;
		// int iLen = 4;
		 setsockopt(sockid,SOL_SOCKET,SO_SNDTIMEO, (char*)&nRecvBuf,len);
		 setsockopt(sockid,SOL_SOCKET,SO_RCVTIMEO,(char*)&nRecvBuf,len);
	
		fd_set set;
		printf("---begin connect \n");
		//重复连接3次，防止中断等原因导致的异常
		for(int i=0;i<3;i++)
		{
			bool ret = false;
			int iret = -1;
			iret = connect(sockid,(struct sockaddr *)&sss_addr,(int)(sizeof(sss_addr)));
			if(iret == -1)
			{
				struct timeval tm;
				tm.tv_sec  = 0;
				tm.tv_usec = 1000*500;
				FD_ZERO(&set);
				FD_SET(sockid, &set);
				int error=-1;
				if( select(sockid+1, NULL, &set, NULL, &tm) > 0)
				{
					getsockopt(sockid, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
					if(error == 0) 
					{
						ret = true;
						printf("---error no \n");
						//break;
					}
					else ret = false;
				} else ret = false;
			}
			else if(iret==0)
			{	
				
				break;
			}
			if(i==2) 
			 {
				  //cout<<"connect Error "<<p_host<<":"<<p_Port<<endl;
				printf("conncet to %s port %d \n",phostIp,iPort);
				//  perror("::connect  ");
				  closesocket(sockid);
				  return -1;
			 }
			 printf("---sleeep\n");
			Sleep(1);
			 //cout<<"connect again: "<<p_host<<":"<<p_Port<<endl;
		}
*/		int 	optval = 1;
	
		 ul = 1;
		ioctlsocket(sockid, FIONBIO, &ul); //设置为非阻塞模式

		printf("---connect success %s port %d \n",phostIp,iPort);
		m_clientSocket = sockid;
		return 0;

}

int Stream::ConnectServer_udp(const char* phostIp,int iPort)
{
	sockaddr_in sss_addr;
		int sockid;
		
		sockid=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			
		sss_addr.sin_family = AF_INET ;
	
		sss_addr.sin_addr.s_addr = inet_addr(phostIp) ;
		sss_addr.sin_port=htons((unsigned short)iPort);
		strcpy(m_strdstIP,phostIp);
		m_idstport  = iPort;

		 unsigned long ul = 1;
/*		 ioctlsocket(sockid, FIONBIO, &ul); //设置为非阻塞模式
		 int len = sizeof(int);
		 int nRecvBuf = 3000;
		// int iLen = 4;
		 setsockopt(sockid,SOL_SOCKET,SO_SNDTIMEO, (char*)&nRecvBuf,len);
		 setsockopt(sockid,SOL_SOCKET,SO_RCVTIMEO,(char*)&nRecvBuf,len);
*/
		 /*	
		fd_set set;
		printf("---begin connect \n");
		//重复连接3次，防止中断等原因导致的异常
		for(int i=0;i<3;i++)
		{
			bool ret = false;
			int iret = -1;
			iret = connect(sockid,(struct sockaddr *)&sss_addr,(int)(sizeof(sss_addr)));
			if(iret == -1)
			{
				struct timeval tm;
				tm.tv_sec  = 0;
				tm.tv_usec = 1000*500;
				FD_ZERO(&set);
				FD_SET(sockid, &set);
				int error=-1;
				if( select(sockid+1, NULL, &set, NULL, &tm) > 0)
				{
					getsockopt(sockid, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
					if(error == 0) 
					{
						ret = true;
						printf("---error no \n");
						//break;
					}
					else ret = false;
				} else ret = false;
			}
			else if(iret==0)
			{	
				
				break;
			}
			if(i==2) 
			 {
				  //cout<<"connect Error "<<p_host<<":"<<p_Port<<endl;
				printf("conncet to %s port %d \n",phostIp,iPort);
				//  perror("::connect  ");
				  closesocket(sockid);
				  return -1;
			 }
			 printf("---sleeep\n");
			Sleep(1);
			 //cout<<"connect again: "<<p_host<<":"<<p_Port<<endl;
		}
*/
		int 	optval = 1;
	
		ul = 0;
		ioctlsocket(sockid, FIONBIO, &ul); //设置为阻塞模式
	//	printf("---connect success %s port %d \n",phostIp,iPort);

		m_clientSocket = sockid;
		return 0;

}
