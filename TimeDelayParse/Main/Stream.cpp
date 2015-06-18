#include "Stream.h"
#include "cJSON.h"


Stream::Stream()
{

	pRet_root = NULL;
	m_clientSocket = -1;
	m_idstport = 0;
	memset(m_strdstIP,0,sizeof(m_strdstIP));
	memset(m_Url,0,sizeof(m_Url));
	
}
Stream::~Stream()
{
	if(m_clientSocket != -1)
		close(m_clientSocket);
	
	m_clientSocket = -1;
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

	memset(alivetick,0,sizeof(alivetick));
	int length = 0;
	int i_rc = 0, i_count = 0;
	int iRecvLen = 0;
	do
	{
		i_rc = recv(m_clientSocket, alivetick + i_count, 2000 - i_count, 0);
		if (i_rc <= 0)break;//“Ï≥£πÿ±’
		i_count += i_rc;
	} while (strstr(alivetick, "XXEE") == NULL);
	iRecvLen = i_count;
	if (iRecvLen <= 0){
		::close(m_clientSocket);
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


bool Stream::Send_Jsoon_str()
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

	
	if(m_clientSocket != -1)
	{
		send(m_clientSocket, cJsonBuff, strlen(cJsonBuff), 0);
		printf("---send len=%d\n",strlen(cJsonBuff));
		printf("---send over\n");
		fflush(stdout);

	}
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

			//≈–∂œ«Î«Û¿‡–
			if (strcmp(pcmd->valuestring, "login") == 0)
			{
				//ªÒ»°∑µªÿ¬Î
				cJSON* pRetCode = cJSON_GetObjectItem(pRoot, "retcode");
				char strretcode[128]={0};
				memcpy(strretcode,pRetCode->valuestring,strlen(pRetCode->valuestring)+1);
				if(atoi(strretcode) >= 0)
				{
					//success
					
				}
				else
				{
					//failed
				}
			
			}
			else if (strcmp(pcmd->valuestring, "logout") == 0)
			{

			}
			//◊¥Ã¨º‡≤‚‘› ±Œﬁ
				
		}
	}
	
	return true;
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

int Stream::ConnectServer(const char* phostIp,int iPort)
{
		struct sockaddr_in s_addr;
		int sockid;
		socklen_t addr_len;
	
		sockid=socket(AF_INET,SOCK_STREAM,0);
			
		s_addr.sin_family = AF_INET ;
	
		s_addr.sin_addr.s_addr = inet_addr(phostIp) ;
		s_addr.sin_port=htons((unsigned short)iPort);
	
	   
	/*	 if (-1 == fcntl(sockid, F_SETFL, O_NONBLOCK))
		 {
			 printf("fcntl socket error!\n");
			 fflush(stdout);
			 return -1;
		 }	 
	*/
		 
		 unsigned long ul = 1;
		 ioctl(sockid, FIONBIO, &ul); //…Ë÷√Œ™∑«◊Ë»˚ƒ£ Ω
		 struct timeval timeout={3,1000*500}; //3√Î
		 int len = sizeof(timeout);
		 setsockopt(sockid,SOL_SOCKET,SO_SNDTIMEO, &timeout,len);
		 setsockopt(sockid,SOL_SOCKET,SO_RCVTIMEO,&timeout,len);
	
		fd_set set;
		printf("---begin connect \n");
		//÷ÿ∏¥¡¨Ω”3¥Œ£¨∑¿÷π÷–∂œµ»‘≠“Úµº÷¬µƒ“Ï≥£
		for(int i=0;i<3;i++)
		{
			bool ret = false;
			int iret = -1;
			iret = connect(sockid,(struct sockaddr *)&s_addr,(int)(sizeof(s_addr)));
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
					getsockopt(sockid, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
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
				  perror("::connect  ");
				  ::close(sockid);
				  return -1;
			 }
			 printf("---sleeep\n");
			 usleep(1000);
			 //cout<<"connect again: "<<p_host<<":"<<p_Port<<endl;
		}
		int 	optval = 1;
	
			ul = 0;
		ioctl(sockid, FIONBIO, &ul); //…Ë÷√Œ™◊Ë»˚ƒ£ Ω
		printf("---connect success %s port %d \n",phostIp,iPort);

		m_clientSocket = sockid;
		return 0;

}



