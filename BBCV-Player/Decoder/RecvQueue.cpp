
#include "RecvQueue.h"
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/time.h>
//#include <unistd.h>
//#include <time.h>
#include <process.h>

const int TS_PACKET_SIZE = 188;

FILE *fpdumxer = NULL;

//#pragma comment(lib,"winsock.lib")
#pragma comment(lib,"ws2_32.lib")



NewQueue::NewQueue(int iport)
{
	bufsize = 0;
	write_ptr = 0;
	read_ptr = 0;
	buf = NULL;
	m_iport = iport;
	m_boverlay = false;
	m_hsIDRFrame = false;
	m_bIsOverlaying = false;
	m_bInitDecoder = false;
	m_bDelayFrame = false;

	m_iperiod = 1000;
	m_iRate = 0;
	m_iPMTPID = 0;
	m_iVideoPID = 0;
	m_iAudioPID = 0;
	m_iPCRPID = 0;

	m_llDts = 0;
	m_llPts = 0;
	m_llLastPts = 0;
	m_iFrameRate = 0;
	m_iGopSize = 0;
	m_iFramTotal = 0;

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
	//sem_init(&m_sem_send,0,0);

	InitializeCriticalSection(&locker);

	//if(NULL == fpdumxer)
	//	fpdumxer = fopen("dumxer.pes","w+b");
}

NewQueue::~NewQueue()
{
	int iloop = 5;
	while(iloop-- && udp_recv_thread !=0)
	{
		m_bstop = true;
		Sleep(1000);
	}
	clean_RecvQue();
	free_queue();

}

int NewQueue::FilterRTPData(char* buff,int ilen)
{
	bool ret = false;
	//rtsp 则过滤rtp头
	int iFilterLen = 0;
	RTPHead tmRtpHead;
	memset(&tmRtpHead,0,sizeof(tmRtpHead));
	unsigned char firstChar = *buff;
	tmRtpHead.Version = firstChar>>6 ;
	tmRtpHead.PayloadFlag = (firstChar<<2) >>7;
	tmRtpHead.ExternData =	(firstChar<<3) >>7;
	tmRtpHead.CSRCCount = (firstChar<<4) >>4;

	int iCSRCLen = tmRtpHead.CSRCCount * 4;
	iFilterLen = 12 +iCSRCLen;
	int iExterLen = 0;
	if(tmRtpHead.ExternData == 1)
	{
		char *ptmp = buff+iFilterLen;
		int iexterlen = (int)(ptmp[2]<<8);
		iexterlen = iexterlen | ptmp[3] ;
	}

	return iFilterLen;
}

//FILE *fprcv = NULL;
unsigned int _stdcall  NewQueue::udp_ts_recv(void* param)
{
	NewQueue* this0 = (NewQueue*)param;

	this0->udp_recv_thread = GetCurrentThreadId();

	SOCKET sock;
	int addr_len;
	int len;
	char UDP_buf[4096];
	FILE *fp;


	//fp = fopen("out.ts", "wb+");

/*	if ( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))  == -1) {
		//perror("socket");
		//exit(errno);
		printf("create socket.error \n\r");
		return -1;
	} else
		printf("create socket.\n\r");
*/
	sockaddr_in clientVideoService; 
	sockaddr_in c_addr; 
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int iPortAdd = 0;

	sockaddr_in clientVideoService2; 
	memset(&clientVideoService, 0, sizeof(struct sockaddr_in));
	clientVideoService.sin_family = AF_INET;

	clientVideoService.sin_port = htons(this0->m_iport);


	clientVideoService.sin_addr.s_addr = INADDR_ANY;

	sockaddr_in send_addr;
	send_addr.sin_family=AF_INET;
	send_addr.sin_port = htons(this0->m_iSendPort);
	send_addr.sin_addr.s_addr = inet_addr(this0->m_cdstIP);


	int nRecvBuf = 0;
	int iLen = 4;
	setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (char*)&nRecvBuf, sizeof(int) );

	if ( (bind(sock, (struct sockaddr*)&clientVideoService, sizeof(clientVideoService))) == -1 ) {
	//	perror("bind");
	//	exit(errno);
	}else
		printf("bind address to socket.\n\r");

	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nRecvBuf, &iLen);
	nRecvBuf = 1024*1024;//设置为32K
	setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	int nSize = 0;
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, &iLen);


	//printf("begin recv data=========port =%d,nsize=%d \n",this0->m_iport,nSize);
	FILE* fpLog = this0->m_logfp;
	if(fpLog)
	{
		SYSTEMTIME systm;
		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 接收数据端口:%d SO_RCVBUF=%d 缓冲区%d。。。\n",systm.wMinute,systm.wSecond,systm.wMilliseconds,this0->m_iport,nSize,this0->bufsize);
		fflush(fpLog);
	}

	//if(fprcv ==NULL)
	//	fprcv = fopen("rcv.ts","wb");
	
	addr_len = sizeof(struct sockaddr);
	bool bneedwait = false;

	int iFilterLen = 0;
	this0->m_ltotalByte = 0;
	SYSTEMTIME systm1;
	SYSTEMTIME systm2;
	GetLocalTime(&systm1);

	LARGE_INTEGER lpPerformanceCount;
	LARGE_INTEGER lptime1;
	LARGE_INTEGER lptime2;

	QueryPerformanceFrequency(&lpPerformanceCount);
	QueryPerformanceCounter(&lptime1);

	while(!this0->m_bstop) {
		len = recvfrom(sock, UDP_buf, sizeof(UDP_buf)-1, 0, (struct sockaddr*)&c_addr, &addr_len);
		
		//len = recvfrom(sock, UDP_buf, 1500, 0, (struct sockaddr*)&c_addr, &addr_len);
		if (len < 0) {
			//perror("recvfrom");
			//exit(errno);
			printf("recvfrom error \n");
			continue;
		}
		iFilterLen = 0;
		//检测是否RTP数据
		if(this0->m_bNeedControlPlay)
		{
			//rtsp 则过滤rtp头
			iFilterLen = this0->FilterRTPData(UDP_buf,len);
		}
		QueryPerformanceCounter(&lptime2);

	//	fwrite(UDP_buf+iFilterLen, 1, len-iFilterLen, fprcv);
//		if(!this0->m_bNeedControlPlay)
//			this0->ParseStreamInfo((uint8_t*)UDP_buf+iFilterLen,len-iFilterLen);
//		bool bret = this0->Find_Stream_IFrame((uint8_t*)UDP_buf+iFilterLen,len-iFilterLen);
//		if(bret)
		{
			int ggg = 111;
		}
	
		this0->put_queue( (uint8_t*)UDP_buf+iFilterLen, len-iFilterLen);
		this0->m_ltotalByte += len;
		int iIternval = (lptime2.QuadPart-lptime1.QuadPart)*1000/lpPerformanceCount.QuadPart ;
		if(iIternval >= this0->m_iperiod)
		{
			lptime1 = lptime2;
			this0->m_iRate = (this0->m_ltotalByte * 8*1000/iIternval )/1024;
			this0->m_ltotalByte = 0;
		}

	//	this0->m_tsStreamparse.ParseStreamFrame((uint8_t*)UDP_buf+iFilterLen, len-iFilterLen);
		//this0->filterNullPacket(UDP_buf,len);

		//bool bhandle = false;
		//printf("======recv size = %d",len);
//		fwrite(buf, sizeof(char), len, fp);
//		printf("recv from %s:%d,msg len:%d\n\r", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port), len);

/*		if(this0->m_bDelayFrame)
		{
			int ihandleLen = 0;
			bhandle = this0->dumxer(UDP_buf,len,&ihandleLen,1);
			
			if(bhandle)
			{
				printf("------wait a frame delay \n");
				this0->m_bDelayFrame=false;
				int iret = sendto(sock,UDP_buf,len,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
				if(iret < 0)
				{
					perror("sendto");
					exit(errno);
				}
				//后面部分需要送解码
				this0->put_queue( UDP_buf, len);
			}
		}
*/		
/*	
		if((this0->m_boverlay && !this0->m_hsIDRFrame) )//|| bneedwait
		{

			int ihandleLen = 0;
			
			bhandle = this0->dumxer(UDP_buf,len,&ihandleLen);
			if(bhandle)
			{
			//	struct timeval tm;
				
			//	gettimeofday(&tm,NULL);
			//	printf("-----change to overlay =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
				this0->m_bIsOverlaying = true;
				this0->m_hsIDRFrame = true;
				bneedwait = !bneedwait;
				this0->m_bDelayFrame = false;
				printf("------dumxer handle len =%d \n",ihandleLen);
				//fwrite(UDP_buf+ihandleLen,1,len-ihandleLen,fpdumxer);
				//fflush(fpdumxer);

				//fwrite(UDP_buf,1,len,fpdumxer);
				//fflush(fpdumxer);
				//if(!bneedwait)
				{
					//前面部分不是I帧，需要发送出去
					int iret = sendto(sock,UDP_buf,ihandleLen,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
					if(iret < 0)
					{
						perror("sendto");
						exit(errno);
					}
					//后面部分需要送解码
					this0->put_queue( UDP_buf, len);
				}
			//else
			
		
				{
					if(ihandleLen > 0)
					{
						int iret = sendto(sock,UDP_buf,ihandleLen,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
						if(iret < 0)
						{
							perror("sendto");
							exit(errno);
						}
					}
					//后面部分需要送解码
					this0->put_queue( UDP_buf, len);
				}
		
				

			}
			//需要将发送数据与解码数据区分处理
			
		}
		else if((!this0->m_boverlay && this0->m_bIsOverlaying))
		{

			int ihandleLen = 0;
			this0->m_bDelayFrame = true;
			
			bhandle = this0->dumxer(UDP_buf,len,&ihandleLen);
			if(bhandle)
			{
				//需要将发送数据与解码数据区分处理
				this0->m_bIsOverlaying = false;
				this0->m_hsIDRFrame = false;				
				
				bneedwait = !bneedwait;
					//printf("------dumxer handle len =%d \n",ihandleLen);
				//fwrite(UDP_buf+ihandleLen,1,len-ihandleLen,fp);
				//fflush(fp);
				
					//前面部分不是I帧，需要送解码
				this0->put_queue( UDP_buf, ihandleLen);
				//后面部分需要发送出去 
				int iret = sendto(sock,UDP_buf+ihandleLen,len-ihandleLen,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
				if(iret < 0)
				{
					perror("sendto");
					exit(errno);
				}
				


				


				struct timeval tm;

				gettimeofday(&tm,NULL);
				printf("-----change to send  =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
				this0->put_queue( UDP_buf, len);

				//等tssmooth模块发送完整帧结束
				int mswait = 1000*40; //40 ms
				struct timespec ts;                         
	            clock_gettime(CLOCK_REALTIME, &ts );    //获取当前时间
                ts.tv_sec += (mswait / 1000 * 1000 *1000);        //加上等待时间的秒数
                ts.tv_nsec += ( mswait % 1000 ) * 1000; //加上等待时间纳秒数
                int rv = 0;
                rv=sem_timedwait( &this0->m_sem_send, &ts );

			//	sem_wait(&this0->m_sem_send);
				gettimeofday(&tm,NULL);
				printf("-----change to send 222 =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
				//后面部分需要发送出去
				this0->m_bDelayFrame = false;
				int iret = sendto(sock,UDP_buf+ihandleLen,len-ihandleLen,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
				if(iret < 0)
				{
					perror("sendto");
					exit(errno);
				}
					//usleep(5*1000);
			
			}	
		}
			
		if(!bhandle)
		{
			if((this0->m_boverlay && this0->m_hsIDRFrame)  ||(!this0->m_boverlay && this0->m_bIsOverlaying)){
				this0->put_queue( UDP_buf, len);
				if(!this0->m_boverlay && this0->m_bIsOverlaying)
				{
					int iret = sendto(sock,UDP_buf,len,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
					if(iret < 0)
					{
						perror("sendto");
						exit(errno);
					}
				}
			}
			else if(!this0->m_boverlay && !this0->m_bIsOverlaying )
			{
				if(!this0->m_bInitDecoder)
				{
					//printf("--------init data copy\n");
					this0->put_queue( UDP_buf, len);
				}
				int iret = sendto(sock,UDP_buf,len,0,(struct sockaddr*)&send_addr,sizeof(send_addr));
				if(iret < 0)
				{
					perror("sendto");
					exit(errno);
				}
			}
		}

		*/
	}
//	fclose(fp);
	closesocket(sock);
	this0->udp_recv_thread = 0;
	return NULL;
	
}

void NewQueue::init_queue( int size,int  iport,const char* dstip,short isendPort,FILE* fp,FILE* fpInfo,bool bNeedControlPlay)
{
	
	//pthread_cond_init(&cond, NULL);
	buf = (uint8_t*)malloc(sizeof(uint8_t)*size);
	read_ptr = write_ptr = 0;
	bufsize = size;
	m_iport = iport;
	m_iSendPort = isendPort;
	memset(m_cdstIP,0,sizeof(m_cdstIP));
	strcpy(m_cdstIP,dstip);
	m_boverlay = false;
	m_bstop = false;
	m_logfp = fp;
	m_Mediafp = fpInfo;
	m_bNeedControlPlay = bNeedControlPlay;
	
	//
//	pthread_create(&udp_recv_thread, NULL, udp_ts_recv, this);
//	pthread_detach(udp_recv_thread);

	
	ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, udp_ts_recv, (LPVOID)this, 0, &udp_recv_thread);
	CloseHandle (ThreadHandle);
}

void NewQueue::free_queue()
{
	if(buf)
		free(buf);

//	pthread_mutex_destroy(&locker);
	//pthread_cond_destroy(&cond);
	DeleteCriticalSection(&locker);

}



void NewQueue::put_queue(uint8_t* inputbuff, int size)
{
	uint8_t* dst = buf + write_ptr;
	//if(m_boverlay)
	//{
	//	fwrite(inputbuff,1,size,fpdumxer);
	//	fflush(fpdumxer);
	//}

	if((write_ptr > read_ptr && (read_ptr+bufsize < write_ptr+size)) || (write_ptr < read_ptr && (write_ptr+size > read_ptr)))
	//if((write_ptr-read_ptr >= bufsize-1) || (write_ptr <= read_ptr -2))
	{
		printf("====failed write_ptr < readptr\n");
		if(m_logfp)
		{
			SYSTEMTIME systm;
			memset(&systm,0,sizeof(systm));
			GetLocalTime(&systm);
			fprintf(m_logfp,"%2d:%2d:%3d 接收数据====failed write_ptr < readptr \n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(m_logfp);
		}
		Sleep(1);
	}
	//printf("recv_data writelen=%d readlen=%d,buffsize=%d\n",write_ptr,read_ptr,bufsize);
	EnterCriticalSection(&locker);

	if ((write_ptr + size) > bufsize) {
		memcpy(dst, inputbuff,(bufsize - write_ptr));
		memcpy(buf, inputbuff+(bufsize - write_ptr), size-(bufsize - write_ptr));
	} else {
		memcpy(dst, inputbuff, size*sizeof(uint8_t));
	}
	write_ptr = (write_ptr + size) % bufsize;

	//pthread_cond_signal(&cond);
	LeaveCriticalSection(&locker);

}

int NewQueue::get_queue(uint8_t* outbuf, int size)
{
	uint8_t* src = buf + read_ptr;
	int wrap = 0;
	int irealLen = size;


	int pos = write_ptr;
//	if ( (read_ptr + size) > pos)
//	{
//		return -1;
//	}
	if(m_bstop)
		return -1;

	EnterCriticalSection(&locker);


	if (pos < read_ptr) {
		pos += bufsize;
		wrap = 1;
	}

	if ( (read_ptr + size) > pos) {
		//memcpy(outbuf, src, (bufsize - read_ptr));
		//printf("read recv data failed \n");
		LeaveCriticalSection(&locker);
		return -1;
//		struct timespec timeout;
//		timeout.tv_sec=time(0)+1;
//		timeout.tv_nsec=0;
//		pthread_cond_timedwait(&que->cond, &que->locker, &timeout);
//		if ( (que->read_ptr + size) > pos ) {
//			pthread_mutex_unlock(&que->locker);
//			return 1;
//		}
	}

	if (wrap) {
		printf("wrap...\n");
/*		if(m_logfp)
		{
			SYSTEMTIME systm;
			memset(&systm,0,sizeof(systm));
			GetLocalTime(&systm);
			fprintf(m_logfp,"%2d:%2d:%3d 接收数据wrap...\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(m_logfp);
		}*/
		if(size > bufsize - read_ptr)
		{
			memcpy(outbuf, src, (bufsize - read_ptr));
			memcpy(outbuf+(bufsize - read_ptr), buf, size-(bufsize - read_ptr));
		}
		else
		{
			memcpy(outbuf, src, sizeof(uint8_t)*size);

		}
	} else {
		memcpy(outbuf, src, sizeof(uint8_t)*size);
	}
	read_ptr = (read_ptr + size) % bufsize;
	LeaveCriticalSection(&locker);

	return 0;
}


bool NewQueue::set_tsDecoder_stat(bool bstat)
{
	if(m_boverlay != bstat)
	{

		m_boverlay= bstat;

		if(m_boverlay)
		{
			clean_RecvQue();
		}
		struct timeval tm;
		

		//gettimeofday(&tm,NULL);
		//printf("-----Set decoder %d stat =%ld\n",bstat,tm.tv_sec*1000+tm.tv_usec/1000);
		//m_hsIDRFrame = false;
		//m_bFirstDecodeSuccess = false;
	}
	return true;
}

void NewQueue::filterNullPacket(char* buff,int ilen)
{
	
	//识别是否ts头
	int itsHead;
	int index = 0;
	char *packet;
	char filterbuff[4096];

	int ifilterLen = 0; //ts len

	bool hasFindIframe = false;
	bool hasFindSPS = false;
	bool hasFindPPS = false;
	if(ilen <= TS_PACKET_SIZE)
	{
		// to que
		put_queue( (uint8_t*)buff, ilen);
		return ;
	}
	while(index < ilen-TS_PACKET_SIZE)
	{
		if(buff[index]==0x47 && buff[index+TS_PACKET_SIZE]==0x47)
		{
			//printf("-----find a ts packet \n");
			packet = &buff[index];
			int len, pid, cc,  afc, is_start, is_discontinuity,
				has_adaptation, has_payload;
			unsigned short *sPID =  (unsigned short*)(packet + 1);
			pid = ntohs(*sPID)& 0x1fff;
			if(pid != 0x1fff)
			{
				//packet 
				memcpy(filterbuff+ifilterLen,packet,TS_PACKET_SIZE);
				ifilterLen += TS_PACKET_SIZE;
					
			}
				//printf("-------pid=%d\n",pid);
			
			index += TS_PACKET_SIZE;
		}
		else
		{
			index++;
		}

	}
	
	//
	if(ilen - index > 0)
	{
		memcpy(filterbuff+ifilterLen,buf+index,ilen - index);
		ifilterLen += (ilen - index);
	}

	// to que
	put_queue( (uint8_t*)filterbuff, ifilterLen);

}

bool NewQueue::dumxer(unsigned char * buff,int ilen,int *iHandleLen,int ineedflag)
{

	
	//识别是否ts头
	int itsHead;
	int index = 0;
	unsigned char *packet;
	const uint8_t *pESH264_IDR= NULL;

	int iseekLen = 0; //ts head len
	int iPesHeadLen = 0; //pes head len
	int iEsSeekLen = 0;
	int ifindLen =0;

	bool hasFindIframe = false;
	bool hasFindSPS = false;
	bool hasFindPPS = false;
	while(index < ilen && !hasFindIframe)
	{
		if(buff[index]==0x47 && buff[index+TS_PACKET_SIZE]==0x47)
		{
			hasFindIframe = false;
			hasFindSPS = false;
			hasFindPPS = false;
			//printf("-----find a ts packet \n");
			packet = &buff[index];
			if(packet[1] & 0x40)  //is_start = packet[1] & 0x40;
			{
				int len, pid, cc,  afc, is_start, is_discontinuity,
					has_adaptation, has_payload;
				unsigned short *sPID =  (unsigned short*)(packet + 1);
				pid = ntohs(*sPID)& 0x1fff;
				//printf("-------pid=%d\n",pid);
				

				afc = (packet[3] >> 4) & 3;//adaption
				
				if (afc == 0) /* reserved value */
					return 0;
				has_adaptation = afc & 2;
				has_payload = afc & 1;
				is_discontinuity = has_adaptation
					&& packet[4] != 0 /* with length > 0 */
					&& (packet[5] & 0x80); /* and discontinuity indicated */

				cc = (packet[3] & 0xf);
				const uint8_t *p, *p_end,*pEsData;

				p = packet + 4;
				if (has_adaptation) {
					/* skip adaptation field */
					p += p[0] + 1;
					/* if past the end of packet, ignore */
					p_end = packet + TS_PACKET_SIZE;
					if (p >= p_end)
						return 0;
				}
				//pes
				iseekLen = p - packet;
				if(p[0]==0x00&&p[1]==0x00&&p[2]==0x01&&p[3]==0xE0)
				{
					//pes len
					int ilen = p[4]<<8;
					ilen = ilen | p[5];
					//printf("-----------pes len =%d\n",ilen);

					iPesHeadLen=p[8] + 9;
					//printf("--------------pesheadlen=%d \n",iPesHeadLen);
					pEsData = p+iPesHeadLen;
					//
					//find 00000001 7 8 5
					int itempLen = TS_PACKET_SIZE - iseekLen - iPesHeadLen;
					
					const uint8_t *pData= NULL;
					ifindLen = 0;
					while(ifindLen < itempLen)
					{
						pData = pEsData + ifindLen;
						if(*pData==0x00 && *(pData+1)==0x00&& *(pData+2)==0x00 && *(pData+3)==0x01 )
						{
								printf("------------4 0 flag bits =%0x\n",*(pData+4));
								int iflag = (*(pData+4))&0x1f;
								//printf("---------h264 nal type =%d \n",iflag);

							/*	if(iflag == ineedflag)
								{
									printf("----------find need flag \n");
									pESH264_IDR = pData - 5;
									hasFindIframe = true;
									hasFindSPS = true;
									hasFindPPS = true;
									break;
								}
						*/		
								if(iflag == 7)
								{
									printf("----------find sps \n");
									pESH264_IDR = pData - 5;
									hasFindSPS = true;
									//break;
								}
								else if(iflag == 8)
								{
									printf("-------find pps \n");
									pESH264_IDR = pData - 5;
									hasFindPPS = true;
									//break;
									
								}
								else if(iflag == 5)
								{
									printf("-------find IDR \n");
									pESH264_IDR = pData - 5;
									hasFindIframe = true;
									//break;
								}
								if(hasFindIframe && hasFindPPS && hasFindSPS)
								{
									break;
								}
								ifindLen += 5;
							
						}
						else if(*pData==0x00 && *(pData+1)==0x00&& *(pData+2)==0x01 )
						{
								printf("------------3 0 flag bits =%0x\n",*(pData+3));
								int iflag = (*(pData+3))&0x1f;
								//printf("---------h264 nal type =%d \n",iflag);

							/*	if(iflag == ineedflag)
								{
									printf("----------find need flag \n");
									pESH264_IDR = pData - 4;
									hasFindIframe = true;
									break;
								}
								*/
								if(iflag == 7)
								{
									printf("----------find sps \n");
									pESH264_IDR = pData - 4;
									hasFindSPS = true;
									//break;
								}
								else if(iflag == 8)
								{
									printf("-------find pps \n");
									pESH264_IDR = pData - 4;
									hasFindPPS = true;
									//break;
									
								}
								else if(iflag == 5)
								{
									printf("-------find IDR \n");
									pESH264_IDR = pData - 4;
									hasFindIframe = true;
									//break;
								}
								if(hasFindIframe && hasFindPPS && hasFindSPS)
								{
									printf("----find all info \n");
									break;
								}
								ifindLen += 4;
									
							
						}
						else
						{
							ifindLen ++;
						}
						
					}
					
				}
			}
			if(!hasFindIframe)
				index += TS_PACKET_SIZE;
		}
		else
		{
			index++;
		}

	}
	if(hasFindIframe && hasFindPPS && hasFindSPS)
	{
		//int iseek_len = index /*+ pESH264_IDR - packet*/;

		printf("--------find IDR SPS  PPS index =%d tsheade len=%d peslen=%d eslen=%d \n",index,iseekLen,iPesHeadLen,ifindLen);
		*iHandleLen = index;
		// less len = ilen - iseek_len;
		//send data buff to iseek_len 

		return true;
	}

	return false;

}

void NewQueue::clean_RecvQue()
{
	EnterCriticalSection(&locker);

	read_ptr = 0;
	write_ptr = 0;
	LeaveCriticalSection(&locker);
}

bool NewQueue::ParseMediaInfo(uint8_t *buff,int ilen)
{

	
	//识别是否ts头
	int itsHead;
	int index = 0;
	unsigned char *packet;
	const uint8_t *pESH264_IDR= NULL;

	int iseekLen = 0; //ts head len
	int iPesHeadLen = 0; //pes head len
	int iEsSeekLen = 0;
	int ifindLen =0;

	bool hasFindIframe = false;
	bool hasFindSPS = false;
	bool hasFindPPS = false;
	while(index < ilen && !hasFindIframe)
	{
		if(buff[index]==0x47 && buff[index+TS_PACKET_SIZE]==0x47)
		{
			hasFindIframe = false;
			hasFindSPS = false;
			hasFindPPS = false;
			//printf("-----find a ts packet \n");
			packet = &buff[index];


			int len, pid, cc,  afc, is_start, is_discontinuity,
				has_adaptation, has_payload;
			unsigned short *sPID =  (unsigned short*)(packet + 1);
			pid = ntohs(*sPID)& 0x1fff;
			if(pid == 0x1fff)
			{
				//NULL packet
				return false;
			}

			if(m_iPMTPID == 0 && pid ==0 )
			{
				// PAT 找到PMT信息
				//8+ 1+1+2+12+16+2+5+1+8+8
				//section_length 
				unsigned char *pat = packet+4;
				uint64_t iSection_length = (uint64_t)(pat[1]&0xf << 8);
				iSection_length |= pat[2];

				pat += 7;
				//section_number;
				uint64_t iSection_number = *pat;

				//节目
				//program_number 16 reserved 3 pid 13
				//默认为一个pmt
				pat += 3;
				//PMT pid
				m_iPMTPID = (int)((pat[0]&0x1f) << 8);
				m_iPMTPID |= pat[1];
				


			}
			else if(pid == m_iPMTPID && (m_iVideoPID ==0 || m_iAudioPID==0))
			{
				//查找pid
				//8 +1+1+2+12+16+2+5+1+8+8
				unsigned char *pat = packet+4;
				uint64_t iSection_length = (uint64_t)(pat[1]&0xf << 8);
				iSection_length |= pat[2];

				pat += 7;
				//section_number;
				uint64_t iSection_number = *pat;

				//节目
				//program_number 16 reserved 3 pid 13
				//默认为一个pmt
				pat += 1;
				//PMT pid
				m_iPCRPID = (int)((pat[0]&0x1f) << 8);
				m_iPCRPID |= pat[1];

				//program info length
				pat += 2;
				int iprogram_info_length =0;
				iprogram_info_length = (int)((pat[0]&0x0f)<<8);
				iprogram_info_length |= pat[1];

				iprogram_info_length += 2;
				pat += iprogram_info_length;
				
				//stream type
				int iStreamType = *pat;
				pat += 1;

				//音频 或视频pid
				int iTmpPid = (int)((pat[0]&0x1f) << 8);
				iTmpPid |= pat[1];

				m_mapStreamPID.insert(MapPIDStreamType::value_type(iStreamType,iTmpPid));

				pat += 2;
				//
				iprogram_info_length =0;
				iprogram_info_length = (int)((pat[0]&0x0f)<<8);
				iprogram_info_length |= pat[1];
				iprogram_info_length += 2;
				pat += iprogram_info_length;




			}

			
			afc = (packet[3] >> 4) & 3;//adaption

			if (afc == 0) /* reserved value */
				return 0;

			has_adaptation = afc & 2;
			has_payload = afc & 1;
			is_discontinuity = has_adaptation
				&& packet[4] != 0 /* with length > 0 */
				&& (packet[5] & 0x80); /* and discontinuity indicated */

			cc = (packet[3] & 0xf);
			const uint8_t *p, *p_end,*pEsData;

			p = packet + 4;
			if (has_adaptation) {
				/* skip adaptation field */
				//p += p[0] + 1;  p[0]为调整字段长度
				int iPCRflag = p[1]&0x10;
				if(iPCRflag ==0x10)
				{
					//pcrflag 
					uint64_t pcr_high =(uint64_t)(p[2] << 25);
					pcr_high = pcr_high | (p[3] << 17);
					pcr_high = pcr_high | (p[4] << 9);
					pcr_high = pcr_high | (p[5] << 1);
					pcr_high = pcr_high | (p[6]&0x80)>>7;

					uint64_t pcr_low = (uint64_t)((p[6]&0x01)<<8);
					pcr_low == pcr_low | p[7];
					
					m_llPCR = pcr_high*300 + pcr_low;

					m_llPCR = m_llPCR /300; //换成90khz
					//33bit base
					
				}
				
				p += p[0] + 1; // p[0]为调整字段长度
				/* if past the end of packet, ignore */
				p_end = packet + TS_PACKET_SIZE;
				if (p >= p_end)
					return 0;
			}
				
			if(packet[1] & 0x40)  //is_start = packet[1] & 0x40;
			{	
				//查找 pes 
				iseekLen = p - packet;
				if(p[0]==0x00&&p[1]==0x00&&p[2]==0x01&&p[3]==0xE0)
				{
					//pes len
					int ilen = p[4]<<8;
					ilen = ilen | p[5];
					//printf("-----------pes len =%d\n",ilen);

					iPesHeadLen=p[8] + 9;
					//printf("--------------pesheadlen=%d \n",iPesHeadLen);
					pEsData = p+iPesHeadLen;
					//
					//find 00000001 7 8 5
					int itempLen = TS_PACKET_SIZE - iseekLen - iPesHeadLen;
					
					const uint8_t *pData= NULL;
					ifindLen = 0;
					while(ifindLen < itempLen)
					{
						pData = pEsData + ifindLen;
						if(*pData==0x00 && *(pData+1)==0x00&& *(pData+2)==0x00 && *(pData+3)==0x01 )
						{
								printf("------------4 0 flag bits =%0x\n",*(pData+4));
								int iflag = (*(pData+4))&0x1f;
							
								if(iflag == 7)
								{
									printf("----------find sps \n");
									pESH264_IDR = pData - 5;
									hasFindSPS = true;
									//break;
								}
								else if(iflag == 8)
								{
									printf("-------find pps \n");
									pESH264_IDR = pData - 5;
									hasFindPPS = true;
									//break;
									
								}
								else if(iflag == 5)
								{
									printf("-------find IDR \n");
									pESH264_IDR = pData - 5;
									hasFindIframe = true;
									//break;
								}
								if(hasFindIframe && hasFindPPS && hasFindSPS)
								{
									break;
								}
								ifindLen += 5;
							
						}
						else if(*pData==0x00 && *(pData+1)==0x00&& *(pData+2)==0x01 )
						{
								printf("------------3 0 flag bits =%0x\n",*(pData+3));
								int iflag = (*(pData+3))&0x1f;
								//printf("---------h264 nal type =%d \n",iflag);

								if(iflag == 7)
								{
									printf("----------find sps \n");
									pESH264_IDR = pData - 4;
									hasFindSPS = true;
									//break;
								}
								else if(iflag == 8)
								{
									printf("-------find pps \n");
									pESH264_IDR = pData - 4;
									hasFindPPS = true;
									//break;
									
								}
								else if(iflag == 5)
								{
									printf("-------find IDR \n");
									pESH264_IDR = pData - 4;
									hasFindIframe = true;
									//break;
								}
								if(hasFindIframe && hasFindPPS && hasFindSPS)
								{
									printf("----find all info \n");
									break;
								}
								ifindLen += 4;
									
							
						}
						else
						{
							ifindLen ++;
						}
						
					}
					
				}


			}
	
			if(!hasFindIframe)
				index += TS_PACKET_SIZE;
		}
		else
		{
			index++;
		}

	}

	return true;
}

void  NewQueue::Adjust_PMT_table(TS_PMT* packet ,unsigned char *buffer)
{
	int pos=12,len = 0;
	int i = 0;
	packet->table_id = buffer[0];
	packet->section_syntax_indicator = buffer[1]>>7;
	packet->zero = buffer[1]>>6 & 0x01;
	packet->reserved_1 = buffer[1]>>4 & 0x3;
	packet->section_length = (buffer[1]&0x0F) <<8 | buffer[2];
	packet->program_number = buffer[3] <<8 |buffer[4];
	packet->reserved_2 = buffer[5]>>6;
	packet->version_number = buffer[5]>>1 & 0x1F;
	packet->current_next_indicator = (buffer[5]<<7)>>7;
	packet->section_number = buffer[6];
	packet->last_section_number = buffer[7];
	packet->reserved_3 = buffer[8]>>5;

	packet->PCR_PID = ((buffer[8]<<8)|buffer[9]) & 0x1FFF;
	packet->reserved_4 = buffer[10]>>4;

	packet->program_info_length = (buffer[10] & 0x0F)<<8 | buffer[11];

	len = 3+packet->section_length;
	packet->CRC_32 = (buffer[len-4]&0x000000FF) <<24
		| (buffer[len-3] & 0x000000FF) << 16
		| (buffer[len-2] & 0x000000FF) << 8
		| (buffer[len-1] & 0x000000FF);

	if(packet->program_info_length != 0)
		pos += packet->program_info_length;

	for(;pos<=(packet->section_length+2)-4;)
	{
		packet->stream_type = buffer[pos];
		packet->reserved_5 = buffer[pos+1]>>5;
		packet->elementary_PID = ((buffer[pos+1] << 8) | buffer[pos+2])&0x1fff;
		packet->reserved_6 = buffer[pos+3] >> 4;
		packet->ES_info_length = (buffer[pos+3]&0x0F) << 8 | buffer[pos+4];
		
		m_mapStreamPID.insert(MapPIDStreamType::value_type(packet->stream_type,packet->elementary_PID));

		if(packet->stream_type == 0x1B)
		{
			// video
			m_iVideoPID = packet->elementary_PID;
		}
		else if(packet->stream_type == 0x03)
		{
			m_iAudioPID = packet->elementary_PID;
		}

		if(packet->ES_info_length !=0)
		{
			pos += 5;
			pos += packet->ES_info_length;

		}
		else
			pos += 5;
	}

}

int  NewQueue::Adjust_PAT_table(TS_PAT* packet ,unsigned char *buffer)
{
	int n=0,i=0;
	int len = 0;
	packet->table_id = buffer[0];
	packet->section_syntax_indicator = buffer[1]>>7;
	packet->zero = buffer[1]>>6 & 0x01;
	packet->reserved_1 = buffer[1]>>4 & 0x3;
	packet->section_length = (buffer[1]&0x0F) <<8 | buffer[2];
	packet->transport_stream_id = buffer[3] <<8 |buffer[4];
	packet->reserved_2 = buffer[5]>>6;
	packet->version_number = buffer[5]>>1 & 0x1F;
	packet->current_next_indicator = (buffer[5]<<7)>>7;
	packet->section_number = buffer[6];
	packet->last_section_number = buffer[7];
	if(packet->section_length >= TS_PACKET_SIZE-5)
		return -1;

	len = 3+packet->section_length;
	packet->CRC_32 = (buffer[len-4]&0x000000FF) <<24
					| (buffer[len-3] & 0x000000FF) << 16
					| (buffer[len-2] & 0x000000FF) << 8
					| (buffer[len-1] & 0x000000FF);

	for(n=0;n<packet->section_length-4;n++)
	{
		packet->program_number = buffer[8]<<8 |buffer[9];
		packet->reserved_3 = buffer[10]>>5;
		if(packet->program_number == 0x0)
		{
			packet->network_PID = (buffer[10]<<3)<<5|buffer[11];
			//packet->network_PID = (buffer[11]<<3)<<5|buffer[12];
		}
		else
			packet->program_map_PID = (buffer[10]<<3)<<5 |buffer[11];
		
		n += 5;
	}

	return 0;
}
void NewQueue::Adjust_TS_packet_header(TS_packet_Header* pHeader,unsigned char *buffer)
{
	if(pHeader ==NULL)
		return ;
	pHeader->transport_error_indicator = buffer[1] >> 7;
	pHeader->payload_unit_start_indicator = buffer[1]>>6 &0x01;
	pHeader->transport_prority = buffer[1] >> 5 & 0x01;
	pHeader->PID  = (buffer[1] &0x1F)<<8 | buffer[2];
	pHeader->transport_scrambling_control = buffer[3]>>6;
	pHeader->adaption_field_control = buffer[3] >> 4 & 0x03;
	pHeader->continuity_counter = buffer[3] & 0x03;

}


uint64_t NewQueue::Parse_PTS(unsigned char *pBuf)
{
	unsigned long long llpts = (((unsigned long long)(pBuf[0] & 0x0E)) << 29)
		| (unsigned long long)(pBuf[1] << 22)
		| (((unsigned long long)(pBuf[2] & 0xFE)) << 14)
		| (unsigned long long)(pBuf[3] << 7)
		| (unsigned long long)(pBuf[4] >> 1);
	return llpts;
}

bool NewQueue::GetVideoESInfo(unsigned char *pEsData,int itempLen)
{
	int ifindLen = 0;
	const uint8_t *pData= NULL;
	const uint8_t *pESH264_IDR= NULL;

	bool hasFindIframe = false;
	bool hasFindPFrame = false;
	bool hasFindPPS = false;
	bool hasFindSPS = false;
	while(ifindLen < itempLen)
	{
		pData = pEsData + ifindLen;
		if(*pData==0x00 && *(pData+1)==0x00&& *(pData+2)==0x00 && *(pData+3)==0x01 )
		{
			printf("------------4 0 flag bits =%0x\n",*(pData+4));
			int iflag = (*(pData+4))&0x1f;

			if(iflag == 7)
			{
				printf("----------find sps \n");
				pESH264_IDR = pData - 5;
				hasFindSPS = true;
				//break;
			}
			else if(iflag == 8)
			{
				printf("-------find pps \n");
				pESH264_IDR = pData - 5;
				hasFindPPS = true;
				//break;

			}
			else if(iflag == 5)
			{
				printf("-------find IDR \n");
				pESH264_IDR = pData - 5;
				hasFindIframe = true;
			//	m_iGopSize++;
				//break;
			}
			else if(iflag == 1)
			{
				printf("-------find P frame or B frame \n");
				pESH264_IDR = pData - 5;
				hasFindPFrame = true;
				//break;
				m_iGopSize++;
				break;
			}
	
			if(hasFindIframe && hasFindSPS && hasFindIframe)
			{
				printf("----find all info \n");
				//将gopsize 清空
				if(m_iGopSize != 0)
				{
					//记录gopsize
					fprintf(m_Mediafp,"Gop size=%d frametotal=%d\n",m_iGopSize,m_iFramTotal);
					fflush(m_Mediafp);
				}
				m_iGopSize = 0;
				m_iFramTotal = 0;
				break;
			}
			ifindLen += 5;

		}
		else if(*pData==0x00 && *(pData+1)==0x00&& *(pData+2)==0x01 )
		{
			printf("------------3 0 flag bits =%0x\n",*(pData+3));
			int iflag = (*(pData+3))&0x1f;
			//printf("---------h264 nal type =%d \n",iflagr);

			if(iflag == 7)
			{
				printf("----------find sps \n");
				pESH264_IDR = pData - 4;
				hasFindSPS = true;
				//break;
			}
			else if(iflag == 8)
			{
				printf("-------find pps \n");
				pESH264_IDR = pData - 4;
				hasFindPPS = true;
				//break;

			}
			else if(iflag == 5)
			{
				printf("-------find IDR \n");
				pESH264_IDR = pData - 4;
				//m_iGopSize++;
				hasFindIframe = true;
				//break;
			}
			else if(iflag == 1)
			{
				printf("-------find P frame or B frame \n");
				pESH264_IDR = pData - 4;
				hasFindPFrame = true;
				//break;
				m_iGopSize++;
				break;
			}

			if( hasFindPPS && hasFindSPS && hasFindIframe )
			{
				printf("----find all info \n");
				m_iGopSize++;
				//将gopsize 清空
				if(m_iGopSize != 0)
				{
					//记录gopsize
					fprintf(m_Mediafp,"Gop size=%d frametotal=%d\n",m_iGopSize,m_iFramTotal);
					fflush(m_Mediafp);
				}
				m_iGopSize = 0;
				m_iFramTotal = 0;
				break;
			}
			ifindLen += 4;


		}
		else
		{
			ifindLen ++;
		}

	}
	if(hasFindIframe || hasFindPFrame)
		return true;
	else
		return false;

}
bool NewQueue::Adjust_PES_Pakcet(unsigned char *p,int iseekLen)
{
	unsigned char *pEsData;
	if(p[0]==0x00&&p[1]==0x00&&p[2]==0x01)
	{
		if(p[3]==0xE0)
		{
			//video
			
			
		}
		else if(p[3]==0xC0)
		{
			//audio
			int i = 1;
			return false;
		}
		else
		{
			return false;
		}

		//pes len video
		int ilen = p[4]<<8 | p[5];
		//printf("-----------pes len =%d\n",ilen);
		int iptsflag = p[7]>>6;
		m_bHasDTS = false;
		m_bHasPTS = false;
		if(iptsflag == 3)
		{
			//both pts dts;
			m_bHasDTS = true;
			m_bHasPTS = true;
		}
		else if(iptsflag == 2)
		{
			m_bHasPTS = true;
		}
		else
		{
			// 01 forbidden 00 both no
		}
		if(m_bHasPTS)
		{
			m_llPts = Parse_PTS(p+9);
			m_iFramTotal++;
			fprintf(m_Mediafp,"PCR=%d \n",m_llPCR);
			fprintf(m_Mediafp,"PTS=%d \n",m_llPts);
			fprintf(m_Mediafp,"PTS-PCR=%d \n",m_llPts-m_llPCR);
		//	if(p[3]==0xE0)
			{
				double fps = 1000/((m_llPts - m_llLastPts)/90);
				if(m_iFrameRate != fps && m_llLastPts != 0 && fps <= 30 && fps >=15)
				{
					fprintf(m_Mediafp,"rate=%3.1f \n",fps);
					m_iFrameRate = fps;
				}
				m_llLastPts = m_llPts;
			}

			//fflush(m_Mediafp);
		}
		if(m_bHasDTS)
		{
			m_llDts = Parse_PTS(p+9+5);
			fprintf(m_Mediafp,"DTS=%d \n",m_llDts);
			fprintf(m_Mediafp,"DTS-PCR=%d \n",m_llDts-m_llPCR);
			
		}
		fflush(m_Mediafp);
		int iPesHeadLen=p[8] + 9;
		//printf("--------------pesheadlen=%d \n",iPesHeadLen);
		pEsData = p+iPesHeadLen;
		//
		//find 00000001 7 8 5
		int itempLen = TS_PACKET_SIZE - iseekLen - iPesHeadLen;

		//return GetVideoESInfo(pEsData,itempLen);
		return ParseH264ES(pEsData,itempLen);
	}

	return false;
}

//FILE* fptest = NULL;
bool NewQueue::ParseH264ES(unsigned char* buffer,int itemplen)
{
	int buf_index = 0;
	int next_avc = itemplen;
	unsigned char * pData = NULL;
//	if(fptest == NULL)
//		fptest = fopen("test.264","wb+");
	while(1)
	{
		//先找到起始码
		for (; buf_index + 3 < next_avc; buf_index++)
			// This should always succeed in the first iteration.
			if (buffer[buf_index]     == 0 &&
				buffer[buf_index + 1] == 0 &&
				buffer[buf_index + 2] == 1)
				break;
		if (buf_index + 3 >= next_avc)
			return false;

		buf_index += 3;
		//解析当前类型
		pData = buffer + buf_index;
		int iflag = (*(pData))& 0x1f;

		if(iflag == 5)
		{
			// I frame
			m_iGopSize++;
			//将gopsize 清空
			//if(m_iGopSize != 0)
			{
				//记录gopsize
				fprintf(m_Mediafp,"Gop size=%d \n",m_iFramTotal);
				fflush(m_Mediafp);

			//	fwrite(pData,1,itemplen-5-buf_index,fptest);
			}
			m_iGopSize = 0;
			m_iFramTotal = 0;
			break;
		}
		else if(iflag==1)
		{
			m_iGopSize++;
		}


	}


	// 
	return true;
}


int NewQueue::ParseStreamInfo(uint8_t *buff,int ilen)
{

	//识别是否ts头
	int itsHead;
	int index = 0;
	unsigned char *packet;
	const uint8_t *pESH264_IDR= NULL;

	int iseekLen = 0; //ts head len
	int iPesHeadLen = 0; //pes head len
	int iEsSeekLen = 0;
	int ifindLen =0;

	bool hasFindIframe = false;
	bool hasFindSPS = false;
	bool hasFindPPS = false;
	while(index < ilen-TS_PACKET_SIZE && !hasFindIframe)
	{
		if(buff[index]==0x47 && buff[index+TS_PACKET_SIZE]==0x47)
		{
			hasFindIframe = false;
			hasFindSPS = false;
			hasFindPPS = false;
			//printf("-----find a ts packet \n");
			packet = &buff[index];

			TS_packet_Header tmpPacketHeader;
			memset(&tmpPacketHeader,0,sizeof(tmpPacketHeader));
			Adjust_TS_packet_header(&tmpPacketHeader,packet);

			int pid = tmpPacketHeader.PID ;
			int iPoint_fielLen = tmpPacketHeader.payload_unit_start_indicator;

			if(pid == 0x1fff)
			{
				//NULL packet
				return false;
			}
			int len, cc,  afc, is_start, is_discontinuity,
				has_adaptation, has_payload;
			afc = tmpPacketHeader.adaption_field_control;
			uint8_t *p, *p_end,*pEsData;

			int iBeginlen = 4;
			int adaptation_field_length = packet[4];
			switch(tmpPacketHeader.adaption_field_control)
			{
			case 0x0:                                    // reserved for future use by ISO/IEC
				return false;
			case 0x1:                                    // 无调整字段，仅含有效负载  
				//iBeginlen += packet[iBeginlen] + 1;  // + pointer_field
				iBeginlen +=  0;  // + pointer_field
				break;
			case 0x2:                                     // 仅含调整字段，无有效负载
			//	iBeginlen += packet[iBeginlen] + 1;  // + pointer_field
				iBeginlen += 0;  // + pointer_field
				break;
			case 0x3:									 // 调整字段后含有效负载
				if (adaptation_field_length > 0) 
				{
					iBeginlen += 0;                   // adaptation_field_length占8位
					iBeginlen += adaptation_field_length; // + adaptation_field_length
				}
				else
				{
					iBeginlen += 0;
				}
				//iBeginlen += packet[iBeginlen] + 1;           // + pointer_field
				break;
			default:	
				break;

			}

			if(m_iPMTPID == 0 && pid ==0 )
			{
				// PAT 找到PMT信息
				//8+ 1+1+2+12+16+2+5+1+8+8
				//section_length 

				TS_PAT tmpTSPat;
				memset(&tmpTSPat,0,sizeof(tmpTSPat));
				int iret = Adjust_PAT_table(&tmpTSPat,packet+iBeginlen+iPoint_fielLen);  //跳过一个字节指针域
				if(iret < 0)
				{
					// packet not find pid
					return false;
				}

				if(tmpTSPat.program_number == 0x0)
					m_iPMTPID = tmpTSPat.network_PID;
				else
					m_iPMTPID = tmpTSPat.program_map_PID;
				m_iServerPID = tmpTSPat.program_number;
				//记录
				fprintf(m_Mediafp,"PMT PID=%d \n",m_iPMTPID);
				fprintf(m_Mediafp,"Service PID=%d \n",m_iServerPID);
				fflush(m_Mediafp);

			}
			else if(pid == m_iPMTPID && (m_iPCRPID == 0 || m_iVideoPID ==0 || m_iAudioPID==0))
			{
				//查找pid
				TS_PMT tmpPMT;
				memset(&tmpPMT,0,sizeof(tmpPMT));
				Adjust_PMT_table(&tmpPMT,packet+iBeginlen+iPoint_fielLen);  //4
				
				m_iPCRPID = tmpPMT.PCR_PID;

				fprintf(m_Mediafp,"PCR PID=%d \n",m_iPCRPID);
				fprintf(m_Mediafp,"Video PID=%d \n",m_iVideoPID);
				fprintf(m_Mediafp,"Auido PID=%d \n",m_iAudioPID);
				fflush(m_Mediafp);
	/*			MapPIDStreamType::iterator itfind = m_mapStreamPID.begin();
				while(itfind)
				{
					if()
					++itfind;
				}
	*/
			}

			p = packet + 4;
			if (afc == 0) /* reserved value */
				return 0;

			has_adaptation = afc & 2;
			has_payload = afc & 1;
			is_discontinuity = has_adaptation
				&& packet[4] != 0 /* with length > 0 */
				&& (packet[5] & 0x80); /* and discontinuity indicated */

			cc = (packet[3] & 0xf);

			if (has_adaptation) {
				/* skip adaptation field */
				//p += p[0] + 1;  p[0]为调整字段长度
				int iPCRflag = p[1]&0x10;
				if(iPCRflag ==0x10)
				{
					//pcrflag 
					uint64_t pcr_high =(uint64_t)(p[2] << 25);
					pcr_high = pcr_high | (p[3] << 17);
					pcr_high = pcr_high | (p[4] << 9);
					pcr_high = pcr_high | (p[5] << 1);
					pcr_high = pcr_high | (p[6]&0x80)>>7;

					uint64_t pcr_low = (uint64_t)((p[6]&0x01)<<8);
					pcr_low == pcr_low | p[7];

					m_llPCR = pcr_high*300 + pcr_low;

					m_llPCR = m_llPCR /300; //换成90khz
					//33bit base

				//	fprintf(m_Mediafp,"Pcr=%d \n",m_llPCR);
				//	fflush(m_Mediafp);

				}

				p += p[0] + 1; // p[0]为调整字段长度
				/* if past the end of packet, ignore */
				p_end = packet + TS_PACKET_SIZE;
				if (p >= p_end)
					return 0;
			}

			if(packet[1] & 0x40)  //is_start = packet[1] & 0x40;
			{	
				//查找 pes 
				iseekLen = p - packet;
				hasFindIframe = Adjust_PES_Pakcet(p,iseekLen);
				
			}		
			
			if(!hasFindIframe)
				index += TS_PACKET_SIZE;
		}
		else
		{
			index++;
		}

	}	
	return 0;
}



bool NewQueue::Find_Stream_IFrame(unsigned char *buff,int ilen)
{
	//识别是否ts头
	int itsHead;
	int index = 0;
	unsigned char *packet;
	const uint8_t *pESH264_IDR= NULL;

	int iseekLen = 0; //ts head len
	int iPesHeadLen = 0; //pes head len
	int iEsSeekLen = 0;
	int ifindLen =0;

	bool hasFindIframe = false;
	while(index <= ilen-TS_PACKET_SIZE && !hasFindIframe)
	{
		if(buff[index]==0x47)
		{
			//printf("-----find a ts packet \n");
			packet = &buff[index];

			TS_packet_Header tmpPacketHeader;
			memset(&tmpPacketHeader,0,sizeof(tmpPacketHeader));
			Adjust_TS_packet_header(&tmpPacketHeader,packet);

			int pid = tmpPacketHeader.PID ;
			int iPoint_fielLen = tmpPacketHeader.payload_unit_start_indicator;

			if(pid == 0x1fff)
			{
				//NULL packet
				//跳过这个包
				index += TS_PACKET_SIZE;
				continue;
			}
			if (tmpPacketHeader.payload_unit_start_indicator != 0x01) // 表示不 含有PSI或者PES头
			{
				printf("---no palyload data \n");
			//	fflush(stdout);
				//跳过这个包
				index += TS_PACKET_SIZE;
				continue;
			}

			int len, cc,  afc, is_start, is_discontinuity,
				has_adaptation, has_payload;
			afc = tmpPacketHeader.adaption_field_control;
			uint8_t *p, *p_end,*pEsData;


			p = packet + 4;
			has_adaptation = afc & 2;
			has_payload = afc & 1;
			is_discontinuity = has_adaptation
				&& packet[4] != 0 /* with length > 0 */
				&& (packet[5] & 0x80); /* and discontinuity indicated */

			cc = (packet[3] & 0xf);

			if (has_adaptation) {
				/* skip adaptation field */
				//p += p[0] + 1;  p[0]为调整字段长度
				int iPCRflag = p[1]&0x10;

				p += p[0] + 1; // p[0]为调整字段长度
				/* if past the end of packet, ignore */
				p_end = packet + TS_PACKET_SIZE;
				if (p >= p_end)
				{
					//跳过这个包
					index += TS_PACKET_SIZE;
					continue;
				}
			}

			if(packet[1] & 0x40)  //is_start = packet[1] & 0x40; payload_unit_start_indicator
			{	
				//查找 pes 
				iseekLen = p - packet;
				hasFindIframe = Adjust_PES_Pakcet(p,iseekLen);				
			}		

			if(!hasFindIframe)
				index += TS_PACKET_SIZE;
		}
		else
		{
			index++;
		}

	}	



	return hasFindIframe;
}

//设置码率周期
bool NewQueue::Set_tsRate_period(int iperiod)
{
	m_iperiod = iperiod;
	return true;
}

//获取到码率
bool NewQueue::Get_tsRate(int* iRate)
{
	*iRate = m_iRate;
	return true;
}
bool NewQueue::Get_tsIFrame_size(int* iSize)
{
	//m_tsStreamPrase.Get_tsIFrame_size(iSize);
	m_tsStreamparse.Get_tsIFrame_size(iSize);
	return true;
}
