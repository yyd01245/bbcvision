#include "TimeDelayParser.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>

#include "Log.h"


TimeDelayParser::TimeDelayParser()
{
	
}
TimeDelayParser::~TimeDelayParser()
{
	pthread_mutex_destroy(&m_mutex);

}


int TimeDelayParser::Createsocket()
{		
	int sock = -1;
	if ( (sock = socket(AF_INET, SOCK_DGRAM, 0))  == -1) {
		perror("socket");
		fflush(stderr);
		//exit(errno);
		//LOG_ERROR("ERROR  - [SWT]: socket error\n");
		return -1;
	} else
		printf("create socket.\n\r");
	struct sockaddr_in s_addr;
	memset(&s_addr, 0, sizeof(struct sockaddr_in));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(m_iRecvPort);
	s_addr.sin_addr.s_addr = INADDR_ANY;

	int optval = 1;
	if ((setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int))) == -1)
	{
	//	close(sock);
	//	return -1;
		printf("******reuseaddr %d port failed \n",m_iRecvPort);
		fflush(stdout);
		close(sock);
		return -1;
	}


	int nRecvBuf = 0;
	socklen_t iLen = 4;
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &nRecvBuf, &iLen);
	nRecvBuf = 1024*1024;//设置为
//		setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&nRecvBuf,sizeof(socklen_t));
	int nSize = 0;
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &nSize, &iLen);

	printf("---get socket rcvbuf size =%d\n",nSize);

	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &nRecvBuf, &iLen);
	nRecvBuf = 1024*1024;//设置为
//		setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&nRecvBuf,sizeof(socklen_t));
	nSize = 0;
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &nSize, &iLen);
	printf("---get socket sndbuf size =%d\n",nSize);

	
	if ( (bind(sock, (struct sockaddr*)&s_addr, sizeof(s_addr))) == -1 ) {
		perror("bind");
		fflush(stderr);
		printf("******bind %d port failed \n",m_iRecvPort);
		fflush(stdout);
		LOG_ERROR("ERROR  - [SWT]: socket Bind error\n");
		return -3;
	}else
	{
		printf("bind success address port =%d to socket.\n\r",m_iRecvPort);
		
	}


	fflush(stdout);
	if (-1 == fcntl(sock, F_SETFL, O_NONBLOCK))
	{
	    printf("fcntl socket error!\n");
		fflush(stdout);
	    return -4;
	}	

	m_BindSocket = sock;

	return 0;

}

void TimeDelayParser::get_time(char*buff,int siz)
{
    char lbuf[32];
    struct tm tmnow;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    localtime_r(&ts.tv_sec,&tmnow);
    strftime(lbuf,31,"%Y/%m/%d-%H:%M:%S",&tmnow);
    snprintf(buff,siz,"%s.%03ld",lbuf,(ts.tv_nsec+500000)/1000000);

}


int TimeDelayParser::init(const char * strUrl,char* strRSMIP,char* iRSMPort,char* weburl)
{
	int iret = -1;
	char strtemp[1024] ={0};
	strcpy(strtemp,strUrl);
	char *strfind = strstr(strtemp,"port=");

	printf("%s \n",strfind);
	strfind += 5;
	printf("%s \n",strfind);
	char* t = strfind;
	while (isdigit(*t) && (*t != '\0'))
	{
		t++;
	}
	if((*t != '\0'))
		iret = -2;

	char strport[128]={0};
	printf("%s \n",strfind);
	strcpy(strport,strfind);
	m_iRecvPort = atoi(strport);
	printf("%s \n",strport);

	//发送报文
	if(strRSMIP !=NULL && iRSMPort!=NULL)
	{
		int iserverport = atoi(iRSMPort);
		Stream ptmpRequest;
		int ret = ptmpRequest.ConnectServer(strRSMIP,iserverport);
		if(ret < 0)
			return -1;
				//cJSON *pRet_root;
		ptmpRequest.pRet_root = cJSON_CreateObject();
		ptmpRequest.Requst_Json_str(2,"cmd","vnclogin");

		char txt[128] ={0};
		srand(time(NULL));
		sprintf(txt,"dfdafd-%d",rand());
		ptmpRequest.Requst_Json_str(2,"resid",txt);

		ptmpRequest.Requst_Json_str(2,"iip","127.0.0.1");
		ptmpRequest.Requst_Json_str(2,"iport",strport);
		ptmpRequest.Requst_Json_str(2,"rate","3072");
		ptmpRequest.Requst_Json_str(2,"url",weburl);
		ptmpRequest.Requst_Json_str(2,"serialno",txt);
		ptmpRequest.Send_Jsoon_str();

		char txtinfo[48] ={0};
		get_time(txtinfo,48);
		printf("::::::::send json msg Time:%s\n",txtinfo);
		

	}

	m_bSwitchFlag = true;
	if(m_iRecvPort < 65565 && m_iRecvPort > 0)
	{
		printf("get url port = %d \n",m_iRecvPort);
		iret = 0;

		//创建ts文件
		m_fpts = NULL;
		//获取当前时间作为后缀
		char tsFilePath[1024] = {0};
	 	time_t now;
	    struct tm tmnow;
	    time(&now);
	    localtime_r(&now,&tmnow);

	    char timestr[32];
	    strftime(timestr,sizeof(timestr),"%Y/%m/%d-%H:%M:%S",&tmnow);
		printf("***** %s  \n",timestr);
		
		sprintf(tsFilePath,"out%d_%d_%d.ts",tmnow.tm_hour,tmnow.tm_min,tmnow.tm_sec);
		printf("%s \n",tsFilePath);
		m_fpts = fopen(tsFilePath,"wb");
		
		//int fd=open(tsFilePath,O_CREAT|O_WRONLY|O_APPEND,0666);
		//创建接收socket
		if(Createsocket() < 0)
		{
			fprintf(stderr,"create socket error ...\n");
			return -1;
		}
		pthread_mutex_init(&m_mutex, NULL);
	

		//创建线程
		pthread_t tcp_recv_thread1;
		pthread_create(&tcp_recv_thread1, NULL, TS_Recv_Thread, this);
		pthread_detach(tcp_recv_thread1);
		
		pthread_t tcp_recv_thread2;
		pthread_create(&tcp_recv_thread2, NULL, Parse_recv_thread, this);
		pthread_detach(tcp_recv_thread2);

	}

	return iret;
}


int TimeDelayParser::WriteTsFile(unsigned char* cData,int ilen)
{

	if(m_fpts != NULL)
	{
		fwrite(cData,1,ilen,m_fpts);
		fflush(m_fpts);
	}
	return 0;
}

bool TimeDelayParser::PutInBuff(char* UDP_buf,int recv_len)
{
	pthread_mutex_lock(&m_mutex);
	if(write_index - read_index + recv_len > buffer_max_size)
	{
		pthread_mutex_unlock(&m_mutex);
		printf("%s %d Waring,buffer is too small,or send is slow,it will reset!\n",__FUNCTION__,__LINE__);

		fflush(stdout);
		//reset
		write_index = 0;
		read_index = 0;
		//
		//continue;
	}
	
	if(buffer_max_size - write_index < recv_len)
	{
		memmove(buffer,buffer+read_index,write_index - read_index);
	
		write_index = write_index - read_index;
		read_index = 0;
	}
	
	memcpy(buffer+write_index,UDP_buf,recv_len);
	write_index += recv_len;
	
	pthread_mutex_unlock(&m_mutex);

	return true;
}

bool TimeDelayParser::GetBuff(char* send_buffer,int *ilen)
{
	int iGetlen = 188;
	pthread_mutex_lock(&m_mutex);
	if(write_index - read_index <= 0)
	{
		pthread_mutex_unlock(&m_mutex);
		//usleep(20*1000);	
	//	continue;
		return false;
	}
	
	if(write_index - read_index >= iGetlen)
	{
		memcpy(send_buffer,buffer+read_index,iGetlen);
		*ilen = iGetlen;
		read_index += iGetlen;
	}
	else
	{
		memcpy(send_buffer,buffer+read_index,write_index - read_index);
		*ilen= write_index - read_index;
		read_index = write_index;
	}
	pthread_mutex_unlock(&m_mutex);

	return true;
}


void *TimeDelayParser::TS_Recv_Thread(void *arg)
{
	TimeDelayParser *this0 = (TimeDelayParser*)arg;
	TimeDelayParser *param = (TimeDelayParser*)arg;
	int bindSock = this0->m_BindSocket;

	fprintf(stderr,"create recv thread success \n");
	fflush(stderr);
	uint8_t UDP_buf[4096];

	struct sockaddr_in c_addr;

	socklen_t addr_len;
	addr_len = sizeof(struct sockaddr);

	//获取当前线程

	fflush(stdout);
//	struct sockaddr_in send_addr;
//	send_addr.sin_family=AF_INET;
//	send_addr.sin_port = htons(this0->m_cSwitchInfo.iDstPort);
//	send_addr.sin_addr.s_addr = inet_addr(this0->m_cSwitchInfo.strDstIP);
	int len = 0;
	this0->m_iCheckflag = -1;

		struct timeval tv1,tv2;
		long long time1,time2;
		long long nsendbytes = 0;
		long long totaltime = 0;
		long now_bit_rate = 0;

		gettimeofday(&tv1, NULL);
		time1 = tv1.tv_sec*1000 + tv1.tv_usec / 1000;

	int iSendTime = 0;
	long long pcr_timebase = -1;
	long long pcr_value =0;
	struct timespec timens1 = {0, 0};
	struct timespec timens2 = {0, 0};

	this0->m_bSwitchFlag =  true;

	this0->buffer_max_size = iDataBuffLen;
	this0->buffer = (char*)malloc(this0->buffer_max_size);

	char info[48] ={0};
	this0->get_time(info,48);
	printf(":::::::Thread create Time:%s\n",info);
	while(1)
	{
				//判断是否是需要接收的数据
		if(!this0->m_bSwitchFlag)
			break;
		//printf("---\n");
		len = recvfrom(bindSock, UDP_buf, sizeof(UDP_buf)-1, 0, (struct sockaddr*)&c_addr, &addr_len);
		if (len <= 0) {
			//printf("---recv len=%d \n",len);
			usleep(50);
			continue;
		}
		if(this0->m_iCheckflag < 0)
		{
			char txtinfo[48] ={0};
			this0->get_time(txtinfo,48);
			printf("::::::::recv data Time:%s\n",txtinfo);
			printf("-----recv data len=%d \n",len);
			
			int iport = ntohs(c_addr.sin_port);
			char srcIPAddr[128]={0};

			strcpy(srcIPAddr,inet_ntoa(c_addr.sin_addr));
			printf("--recv ip %s ,port=%d \n",srcIPAddr,iport);
			//LOG_INFO_FORMAT("INFO  - [SWT]: recv data len=%d ip %s ,port=%d  \n",len,srcIPAddr,iport);
			fflush(stdout);
			this0->m_iCheckflag = 0;
		}

		//解析TS流，判断出一帧完整的视频数据。
		
		if(!this0->PutInBuff((char*)UDP_buf,len))
		{
			printf("----error input buff ,buff size is too small\n");
		}
		
		

	}
	printf("----end recv \n");
	close(bindSock);	
}



void * TimeDelayParser::Parse_recv_thread(void *arg)
{
	TimeDelayParser *this0 = (TimeDelayParser*)arg;
	TimeDelayParser *param = (TimeDelayParser*)arg;

	FILE* fpts = this0->m_fpts;
	fprintf(stderr,"create send Parse_recv_thread success \n");
	fflush(stderr);
	uint8_t UDP_buf[4096];

	struct sockaddr_in c_addr;

	socklen_t addr_len;
	addr_len = sizeof(struct sockaddr);


	struct timeval tv1,tv2;
	long long time1,time2;
	long long nsendbytes = 0;
	long long totaltime = 0;
	long now_bit_rate = 0;

	gettimeofday(&tv1, NULL);
	time1 = tv1.tv_sec*1000 + tv1.tv_usec / 1000;

	//long bit_rate = this0->m_iSendRate;
	while(1)
	{
						//判断是否是需要接收的数据
		if(!this0->m_bSwitchFlag)
			break;
		//取得转发数据
		int len = 0;
		this0->GetBuff((char*)UDP_buf,&len);
		if(len <=0)
		{
			
			usleep(10);
			continue;
		}

		//写文件
		this0->WriteTsFile(UDP_buf,len);
		//解析数据
		this0->ParseTSData(UDP_buf,len);
		
	}
	printf("---end parse \n");
	
}

int TimeDelayParser::ParseTSData(unsigned char* cData,int ilen)
{
	
	return m_tsStreamPrase.ParseStreamFrame(cData,ilen);
}


