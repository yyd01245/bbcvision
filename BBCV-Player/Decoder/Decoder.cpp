#include "Decoder.h"
//#include <math.h>

//#define H264_ONLY

const int Queue_size = 25;
const int Video_Buff_size = 1280*720*3/2;
const int Audio_Buff_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
#define BUF_SIZE  188*7*2
const int TSPACKETLEN = 188;
typedef unsigned char uint8_t;
const int Recv_Queue_size = 1024*1024*10;

const int Use_ffmpeg_recv = 0; // 1为使用ffmpeg接收，0为自己接收数据。	



void LogToService(char *id, const char *format, ...)
{
	{
		char log[2000];
		memset(log,'\0', sizeof(log));
		if(format != NULL)
		{
			va_list mylist;
			va_start(mylist, format);    
			_vsnprintf( log, sizeof(log)-1, format, mylist);
			va_end(mylist);
		}
		log[sizeof(log)-1] = '\0';
	//	fprintf(fpLog,log);
	}
}

TSDecoder_Instance::TSDecoder_Instance()
{
	m_pVideoCodec = NULL;
	m_pAudioCodec = NULL;
	m_pFmt = NULL;
	m_pVideoCodecCtx = NULL;
	m_pAudioCodecCtx = NULL;
	m_pframe = NULL;
	m_iHeight = 0;
	m_iWidht = 0;
	m_videoindex = -1;
	m_audioindex = -1;
	fpLog = NULL;
	m_avbuf = NULL;
	m_MediaInfofp = NULL;

	m_iThreadid = 0;
	m_bstop = false;
	m_ifileType = localfile;
	m_bDecoderFlag = false;
	m_bFirstDecodeSuccess = false;
	m_bNeedControlPlay = false;
	m_bSaveTSStream = false;
	m_tsStreamfp = NULL;

	char Path[512] = {0};
	GetCurrentDirectory(sizeof(Path),Path);
	strcat(Path,"\\\BBCV-Play.log");
	if(NULL == fpLog)
		fpLog = fopen(Path,"w+");

	char mediaPath[512]={0};
	GetCurrentDirectory(sizeof(mediaPath),mediaPath);
	
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	char tmpTime[128]={0};
	sprintf(tmpTime,"\\\%2d-%2d-%2d.log",systm.wHour,systm.wMinute,systm.wSecond);
	strcat(mediaPath,tmpTime);
	strcpy(m_meidaInfoPath,mediaPath);


	if(NULL==m_MediaInfofp)
	{
		m_MediaInfofp = fopen(m_meidaInfoPath,"a+");
	}

	m_lDisplayPort = 0;
	AMP_Display_GetHandle(m_lDisplayPort);
}

TSDecoder_Instance::~TSDecoder_Instance()
{

	m_pAudioPlay->StopPlay();
	int iloop = 5;
	while(iloop-- && m_iThreadid !=0)
	{
		stopDecoder(true);
		Sleep(200);
	}
	fclose(fpLog);
	fpLog = NULL;
	uninit_TS_Decoder();
}

FILE *fpyuv=NULL;
FILE *fpes = NULL;
void Write_esdata(AVPacket *pkt)
{
	if(NULL == fpes)
		fpes = fopen("es.h264","wb");
	fwrite(pkt->data,1,pkt->size,fpes);
	fflush(fpes);
}

unsigned int _stdcall TSDecoder_Instance::decoder_threadFun(void *param)
{
	TSDecoder_Instance* this0 = (TSDecoder_Instance*)param;

	AVCodec *pVideoCodec, *pAudioCodec;
	AVCodecContext *pVideoCodecCtx = NULL;
	AVCodecContext *pAudioCodecCtx = NULL;
	//AVIOContext * pb = NULL;
	//AVInputFormat *piFmt = NULL;
	AVFormatContext *pFmt = NULL;

	this0->m_iThreadid = GetCurrentThreadId();
	//this0->m_iThreadid = pthread_self();
	int ret = -1;
//#ifdef H264_ONLY
	if(!this0->m_bDecoderFlag)//false 指定播放
	{
		ret = this0->initdecoder();
		if(ret < 0)
		{
			printf("initdecoder failed\n");
			return NULL;
		}
		if(this0->open_inputdata())
		{
			this0->m_recvqueue.m_bInitDecoder = true;
			//清空缓冲区 存在一开始就叠加情况

			//this0->m_recvqueue.read_ptr = 0;
			//this0->m_recvqueue.write_ptr = 0;
		//	Sleep(1);
		//	this0->m_recvqueue.clean_RecvQue();
		//	this0->Clean_Video_audioQue();
			printf("------init open inputdat successs\n");
		}
	//#else
	}
	else	//true 自动匹配
	{
		ret = this0->init_open_input();
		if(ret < 0)
		{
			printf("initdecoder failed\n");
			return NULL;
		}
	}
//#endif	

	bool bInitDecodeFlag = false;

	pVideoCodec = this0->m_pVideoCodec;
	pAudioCodec = this0->m_pAudioCodec;
	pFmt = this0->m_pFmt;
	pVideoCodecCtx = this0->m_pVideoCodecCtx;
	pAudioCodecCtx = this0->m_pAudioCodecCtx;

	int videoindex = this0->m_videoindex;
	int audioindex = this0->m_audioindex;

	int got_picture;
	uint8_t *samples = (uint8_t*)this0->m_audioBuff;
	//AVFrame *pframe = avcodec_alloc_frame();

	AVFrame *pframe = this0->pFFCodec->ff_avcodec_alloc_frame();
	this0->m_pframe = pframe;
	AVPacket pkt;
	//av_init_packet(&pkt);
	this0->pFFCodec->ff_av_init_packet(&pkt);

	//struct timeval tv1;
	int64_t start_time,audio_start_time;
	const int FrameInterval = 40;
	bool bStart = true;
	
	int64_t videopts=0;
	int64_t audiopts=0;
	int64_t video_PTS = 0;
	bool firstopen = true;
	int iKeyFrameTime = 0;

	int64_t lastPTS =0;
	start_time = 0;
	audio_start_time = 0;
	int64_t lastAuidoPTS = 0;
	int64_t Audio_PTS = 0;

	SYSTEMTIME systm={0};
//	fpyuv = fopen("audio.ts","w+b");
//	FILE *fpvideo= fopen("audio.pcm","wb+");

	bool bNeedMediaInfo = true;
	int iFramecount =0;
	int iwritetime = 10;
	do
	{
		int iret = this0->pFFAVFormat->ff_av_read_frame(pFmt, &pkt);
		if (iret >= 0) 
		{
			//fwrite(pkt.data,1,pkt.size,fpyuv);
			
			if (pFmt->streams[pkt.stream_index]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
			{
				//if(this0->m_bDecoderFlag)
					//fwrite(pkt.data,1,pkt.size,fpvideo);
			//	printf("-----packet data %0x %0x %0x %0x %0x dts =%ld pts=%ld\n",pkt.data[0+29],pkt.data[1+29],
			//			pkt.data[2+29],pkt.data[3+29],pkt.data[4+29],pkt.dts,pkt.pts);
				if(this0->m_bSmoothPlay)
				{
					int64_t nowtm = GetTickCount();
					//fprintf(fpyuv,"packet dts is = %ld   machine time=%ld \n",(pkt.dts/90),nowtm);
					//if(this0->m_bDecoderFlag)
					//fwrite(pkt.data,1,pkt.size,fpyuv);
					if(start_time == 0 || ((pkt.dts)/90 -lastPTS) > 100)
					{
						start_time = GetTickCount();
						video_PTS = (pkt.dts)/90;
					}
					int iret = (pkt.dts/90 - video_PTS);
					int isleeptm = iret - (GetTickCount() - start_time) - 5;
					while (isleeptm > 0)
					{
						Sleep(isleeptm);
						break;

					} 
					lastPTS = pkt.dts/90;
				}
				iFramecount++;
				//Write_esdata(&pkt);
				this0->pFFCodec->ff_avcodec_decode_video(pVideoCodecCtx, pframe, &got_picture, &pkt);
				
				if (got_picture) 
				{
					int iwidth = pVideoCodecCtx->width;
					int iHeight = pVideoCodecCtx->height;
					//if(bNeedMediaInfo)
					{
						char txt[512]={0};
						sprintf(txt,"width=%d height=%d iFramecount=%d \n",iwidth,iHeight,iFramecount);
						fwrite(txt,1,strlen(txt),this0->m_MediaInfofp);


						fflush(this0->m_MediaInfofp);
						bNeedMediaInfo = false;
					}

					this0->m_iWidht = iwidth;
					this0->m_iHeight = iHeight;

				/*	if(pframe->key_frame)
					{
						struct timeval tm;

						gettimeofday(&tm,NULL);
						printf("-----decode IFrame time  =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
					}
				*/	
					if(pframe->data[0] != NULL && pframe->data[1] != NULL && pframe->data[2] != NULL)
					{
						if(firstopen)
						{
							memset(&systm,0,sizeof(systm));
							GetLocalTime(&systm);
							fprintf(this0->fpLog,"%2d:%2d:%3d 视频数据解码成功\n",systm.wMinute,systm.wSecond,systm.wMilliseconds,pVideoCodecCtx->codec_id,pAudioCodecCtx->codec_id);
							fflush(this0->fpLog);
							firstopen = false;
						}

						int iHight = iHeight;
						int iWidth = iwidth;

						AMP_Display_SetScale(this0->m_lDisplayPort,true);
						AMP_Display_SetParam(this0->m_lDisplayPort,iWidth,iHight);
						int iTemp = this0->m_iCurrentOutBuf;

						EnterCriticalSection(&this0->m_csVideoYUV[iTemp]);
						unsigned char *buff=this0->m_yuvBuff[iTemp];
						memset(buff,0,iWidth*iHeight*3/2);
						for (int i = 0; i < iHight; i++)
						{
							memcpy(buff+i*iWidth, pframe->data[0]+i*pframe->linesize[0], iWidth);
						}
						for (int i=0; i < iHight/2; i++)
						{
							memcpy(buff+iHight*iWidth+i*iWidth/2, pframe->data[1]+i*pframe->linesize[1], iWidth/2);
						}
						for (int i=0; i < iHight/2; i++)
						{
							memcpy(buff+iHight*iWidth*5/4+i*iWidth/2, pframe->data[2]+i*pframe->linesize[2], iWidth/2);
						}
					//	fwrite(buff,1,iHeight*iWidth*3/2,fpvideo);

						LeaveCriticalSection(&this0->m_csVideoYUV[iTemp]);

						this0->m_iCurrentOutBuf = ((this0->m_iCurrentOutBuf == 0) ? 1 : 0);

						AMP_Display_DisplayYUV(this0->m_lDisplayPort,buff,&this0->m_csVideoYUV[iTemp]);
						

						//this0->Push_Video_Data(iwidth,iHeight,pframe,pkt.dts);
						
					}
				}
				else
				{
					char txt[512]={0};
					sprintf(txt,"decoder failed iFramecount=%d \n",iFramecount);
					fwrite(txt,1,strlen(txt),this0->m_MediaInfofp);


					fflush(this0->m_MediaInfofp);
				}
			}
			else if (pFmt->streams[pkt.stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO) 
			{
				//fwrite(pkt.data,1,pkt.size,fpyuv);
				if(this0->m_bSmoothPlay)
				{
					int64_t nowtm = GetTickCount();
					//fprintf(fpyuv,"packet dts is = %ld   machine time=%ld \n",(pkt.dts/90),nowtm);
					//if(this0->m_bDecoderFlag)
					//fwrite(pkt.data,1,pkt.size,fpyuv);
					if(audio_start_time == 0 || ((pkt.pts)/90 -lastAuidoPTS) > 200)
					{
						audio_start_time = GetTickCount();
						Audio_PTS = (pkt.pts)/90;
					}
					int64_t iret = (pkt.pts/90 - Audio_PTS);
					int isleeptm = iret - (GetTickCount() - audio_start_time) - 5;
					while (isleeptm > 0)
					{
						Sleep(isleeptm);
						break;

					} 
					lastAuidoPTS = pkt.pts/90;
				}

				int frame_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;

				int ret = this0->pFFCodec->ff_avcodec_decode_audio(pAudioCodecCtx, (int16_t *)samples, &frame_size, &pkt);
				
				if (ret > 0)
				{
					//printf("audio pts=%d dts =%d \n",pkt.pts,pkt.dts);
					//this0->Push_Audio_Data(samples,frame_size,pkt.dts);
					//fprintf(stderr, "decode one audio frame!\r");
					//play audio
					
					//fwrite(samples,1,frame_size,fpvideo);

					this0->m_pAudioPlay->PlayData(samples,frame_size);

				}
				else
				{
					OutputDebugString("error decode audio-----\n");
				}				
			}
			this0->pFFCodec->ff_av_free_packet(&pkt);
		}
		else
		{
			this0->pFFCodec->ff_av_free_packet(&pkt);
			printf("=========read frame failed %d\n",iret);
		}
	}while(!this0->m_bstop);
	printf("*******decoderThread over!\n");
	this0->m_iThreadid = 0;
}

int TSDecoder_Instance::get_queue(uint8_t* buf, int size) 
{
	int num = size / TSPACKETLEN;
	int index = 0;
	uint8_t * bufhead = buf;
	while(index < num)
	{
		int ret = m_recvqueue.get_queue(buf,TSPACKETLEN);
		if(ret >= 0)
		{
			//分析ts流数据
			m_tsStreamPrase.ParseStreamFrame(buf,TSPACKETLEN);
			buf += TSPACKETLEN;
			++index;
		}
	}
	if(m_bSaveTSStream && size > 0)
	{
		if(NULL == m_tsStreamfp)
		{

			char mediaPath[512]={0};
			GetCurrentDirectory(sizeof(mediaPath),mediaPath);

			SYSTEMTIME systm;
			GetLocalTime(&systm);
			char tmpTime[128]={0};
			sprintf(tmpTime,"\\\%2d-%2d-%2d.ts",systm.wHour,systm.wMinute,systm.wSecond);
			strcat(mediaPath,tmpTime);
	
			m_tsStreamfp = fopen(mediaPath,"wb");
		}
		if(m_tsStreamfp)
		{
			fwrite(bufhead,1,size,m_tsStreamfp);
			fflush(m_tsStreamfp);
		}

	}
	return 0;
}

//FILE* fpreaddata= NULL;
int TSDecoder_Instance::read_data(void *opaque, uint8_t *buf, int buf_size) {
//	UdpParam udphead;
	TSDecoder_Instance* this0 = (TSDecoder_Instance*)opaque;
	int size = buf_size;
	//int size = 2048;
	int ret;
//	if(NULL == fpreaddata)
//		fpreaddata = fopen("readdata.pes","wb+");
//	printf("read data %d\n", buf_size);
	do {
		
		ret = this0->get_queue(buf, size);
		if(ret < 0)
			Sleep(1);
		//printf("-------read_data ret = %d size=%d\n",ret,buf_size);
		//size += ret;
	} while (ret < 0);
	//printf("read data Ok %d\n", buf_size);
//	if(this0->m_bDecoderFlag)
//		fwrite(buf,1,size,fpreaddata);
	return size;
}

bool TSDecoder_Instance::open_inputdata()
{
	AVCodec *pVideoCodec, *pAudioCodec;
	AVInputFormat *piFmt = NULL;


	//struct timeval tm;
	SYSTEMTIME systm;

	//gettimeofday(&tm,NULL);
	//printf("-----init time 1 =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
//#ifdef PROBE_DATA
	// 开始探测码流
	//
	GetLocalTime(&systm);
	fprintf(fpLog,"%2d:%2d:%3d 开始探测码流类型\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
	fflush(fpLog);

	int iloop = 5;
	do
	{
		if (pFFAVFormat->ff_av_probe_input_buffer(m_pb, &piFmt, "", NULL, 0, 188*7) < 0) {
			printf("probe failed!\n");
			Sleep(1);
		} else {
			printf("probe success!\n");
			//printf("format: %s[%s]\n", piFmt->name, piFmt->long_name);
			break;
		}
		
	}while(iloop-- >=0);
	m_pFmt->pb = m_pb;

	memset(&systm,0,sizeof(systm));
	GetLocalTime(&systm);
	fprintf(fpLog,"%2d:%2d:%3d 探测码流成功\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
	fflush(fpLog);

	if (pFFAVFormat->ff_avformat_open_input(&m_pFmt, "", piFmt, NULL) < 0) {
		//printf("avformat open failed.\n");
		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 打开网络数据失败\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
		return -1;
	} else {
		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 打开网络数据成功\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
	}
//#else
/*	// no probe
	if (pFFAVFormat->ff_avformat_open_input(&m_pFmt, m_cfilename, NULL, NULL) < 0) {
		//printf("avformat open failed.\n");
		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 打开网络数据失败\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
		return -1;
	} else {
		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 打开网络数据成功\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
	}
*/
//#endif
	
	//gettimeofday(&tm,NULL);
	//printf("-----loop =%d	init time 2 =%ld\n",10-iloop,tm.tv_sec*1000+tm.tv_usec/1000);

	enum CodecID videoCodeID;
	enum CodecID audioCodeID;

	switch(m_vCodetype)
	{
		case CODE_HD_VIDEO:
		{
			videoCodeID = CODEC_ID_H264;
			break;
		}
		case CODE_SD_VIDEO:
		{
			videoCodeID = CODEC_ID_MPEG2VIDEO;
			break;
		}
		default:
		{
			videoCodeID = CODEC_ID_MPEG2VIDEO;
			break;
		}
	}

	switch(m_aCodeType)
	{
		case CODE_AUIDO_MP2:
		{
			audioCodeID = CODEC_ID_MP2;
			break;
		}
		case CODE_AUIDO_MP3:
		{
			audioCodeID = CODEC_ID_MP3;
			break;
		}
		case CODE_AUDIO_AAC:
		{
			audioCodeID = CODEC_ID_AAC;
			break;
		}
		default:
		{
			audioCodeID = CODEC_ID_MP2;
			break;
		}
	}

/*
	if (pFFAVFormat->ff_av_find_stream_info(m_pFmt) < 0)
	{
		fprintf(stderr, "could not fine stream.\n");
		return -1;
	}

	pVideoCodec = avcodec_find_decoder(m_pVideoCodecCtx->codec_id);
	if (!pVideoCodec) {
		fprintf(stderr, "could not find video decoder!\n");
		return -1;
	}
	if (avcodec_open(m_pVideoCodecCtx, pVideoCodec) < 0) {
		fprintf(stderr, "could not open video codec!\n");
		return -1;
	}


	pAudioCodec = avcodec_find_decoder(m_pAudioCodecCtx->codec_id);
	if (!pAudioCodec) {
		fprintf(stderr, "could not find audio decoder!\n");
		return -1;
	}
	if (avcodec_open(pAudioCodecCtx, pAudioCodec) < 0) {
		fprintf(stderr, "could not open audio codec!\n");
		return -1;
	}
*/	

	pVideoCodec = pFFCodec->ff_avcodec_find_decoder(videoCodeID);
	if (!pVideoCodec) {
		printf("could not find video decoder!\n");
		return false;
	}
	if (pFFCodec->ff_avcodec_open(m_pVideoCodecCtx, pVideoCodec) < 0) {
		printf("could not open video codec!\n");
		return false;
	}


//	printf("**********vidoe eum=%d auido enum=%d*******video type=%d audiotype=%d \n",m_vCodetype,m_aCodeType,videoCodeID,audioCodeID);

	pAudioCodec = pFFCodec->ff_avcodec_find_decoder(audioCodeID);
	if (!pAudioCodec) {
		printf("could not find audio decoder!\n");
		return false;
	}
	if (pFFCodec->ff_avcodec_open(m_pAudioCodecCtx, pAudioCodec) < 0) {
		printf("could not open audio codec!\n");
		return false;
	}
	memset(&systm,0,sizeof(systm));
	GetLocalTime(&systm);
	fprintf(fpLog,"%2d:%2d:%3d 媒体流数据，视频类型：H264, 音频类型 mp2 \n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
	fflush(fpLog);

	m_pVideoCodec = pVideoCodec;
	m_pAudioCodec = pAudioCodec;
	m_pframe = NULL;

	return true;
}

int TSDecoder_Instance::initdecoder()
{
	
//	AVCodec *pVideoCodec, *pAudioCodec;
	AVCodecContext *pVideoCodecCtx = NULL;
	AVCodecContext *pAudioCodecCtx = NULL;
	AVIOContext * pb = NULL;
//	AVInputFormat *piFmt = NULL;
	AVFormatContext *pFmt = NULL;


	struct timeval tm;

	//av_register_all();
	//avformat_network_init();

	std::string strtmp=m_cfilename;
	std::size_t found  = strtmp.find(":");
	if (found != std::string::npos)
		m_ifileType = internetfile;
	else
		m_ifileType = localfile;
	printf("-------------file Type=%d\n",m_ifileType);

	int port = 0;

	{
		//  udp://@:14000
		std::size_t found  = strtmp.find("@:");
		if (found != std::string::npos)
		{
			std::string tm = strtmp.substr(found+2,(strtmp.length()-found-2));
			//printf("-------get string sub %s \n",tm.c_str());	
			port = atoi(tm.c_str());
			printf("=============get port =%d \n",port);
			
		}
	}

		m_recvqueue.init_queue(Recv_Queue_size,port,m_strDstIP,m_iPort,fpLog,m_MediaInfofp);

		uint8_t *buf = (uint8_t*)pFFCodec->ff_av_mallocz(sizeof(uint8_t)*BUF_SIZE);

		
		//pb = avio_alloc_context(buf, BUF_SIZE, 0, this, TSDecoder_Instance::read_data, NULL, NULL);
		pb = pFFAVFormat->ff_avio_alloc_context(buf, BUF_SIZE, 0, this, read_data, NULL, NULL);
		if (!pb) {
			printf("avio alloc failed!\n");
			return -1;
		}

		m_pb = pb;
		pFmt = pFFAVFormat->ff_avformat_alloc_context();

		pVideoCodecCtx = pFFCodec->ff_avcodec_alloc_context();
		pAudioCodecCtx = pFFCodec->ff_avcodec_alloc_context();

		m_pFmt = pFmt;
		m_pVideoCodecCtx = pVideoCodecCtx;
		m_pAudioCodecCtx = pAudioCodecCtx;

		//分辨率 码率 GOP值  AVCodecContext

	
	InitializeCriticalSection(&m_locker);
	InitializeCriticalSection(&m_audioLocker);
	
	initQueue(Queue_size);

	return 0;
}

int TSDecoder_Instance::init_TS_Decoder(const char* cfilename,HWND hwnd,DecoderControlParam dcParam)
{
	strcpy(m_cfilename,cfilename);
	strcpy(m_strDstIP,dcParam.strDstIP);
	m_iPort = dcParam.iport;
	printf("----filename=%s dstip=%s  port=%d \n",m_cfilename,m_strDstIP,m_iPort);
	m_vCodetype=dcParam.vCodetype;
	m_aCodeType=dcParam.aCodetype;
	m_bDecoderFlag = dcParam.bAutoMatch;
	m_bSmoothPlay = dcParam.bSmooth;
	m_bNeedControlPlay = dcParam.bNeedControlPlay;


	pFFCodec = FFCodec::getInstance();
	pFFAVFormat = FFAvformat::getInstance();
	SYSTEMTIME systm;
	memset(&systm,0,sizeof(systm));
	GetLocalTime(&systm);
	fprintf(fpLog,"%2d:%2d:%3d 流地址：%s  初始化开始。。。\n",systm.wMinute,systm.wSecond,systm.wMilliseconds,m_cfilename);
	fflush(fpLog);

	//兼容rtsp
	//URLTYPE m_ifileType= tsUDP;
/*	char strURL[1024]={0};
	strcpy(strURL,m_cfilename);
	int port = 0;
	char cUDPPort[8] = {0};

	char pURL[1024] = {0};
	strcpy(pURL,strURL);
	char *pFind1 = strstr(pURL,"udp:");
	char *pFind2 = strstr(pURL,"rtsp:");
*/
	//  udp://@:14000 rtsp://192.168.20.131:8554/sd.ts
//	if(pFind1 != NULL)
	{
		//ts 流
		m_iURLType = tsUDP;
		//unsigned Decoder_thread;
		//	pthread_create(&Decoder_thread, NULL, TSDecoder_Instance::decoder_threadFun, this);
		//	pthread_detach(Decoder_thread);
		//启动数据回调线程
		HANDLE ThreadHandle;
		ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, decoder_threadFun, (LPVOID)this, 0, &m_iThreadid);
		CloseHandle (ThreadHandle);
	}
#if 0
	else if(pFind2 != NULL)
	{
		//rtsp 流
		m_iURLType = tsRTSP;

		char strRtspIP[256] ={0};
		//获取 ip 端口 资源
		char* pfindport1 = strstr(pURL,"//");
		if(pfindport1 != NULL)
		{

			char* pfindport2 = strstr(pfindport1,":");
			if(pfindport2)
			{
				char strPorttmp[1024] ={0};
				strcpy(strPorttmp,pfindport2);
				

				int ilen = pfindport2 - pfindport1;
				*(pfindport1+ilen) = '\0';
				strcpy(strRtspIP,pfindport1+2);
				char* pfingport3 = strstr(strPorttmp,"/");
				if(pfingport3)
				{
					*pfingport3 = '\0';

					port = atoi(strPorttmp+1);
				}
			}
		}
		//获取rtsp server的ip Port，
		memset(m_strRtspServerIP,0,sizeof(m_strRtspServerIP));
		strcpy(m_strRtspServerIP,strRtspIP);
		m_iRtspPort = port;

		//创建Myrtsp对象

		//开启rtsp连接线程

		//开启rtsp客户端

	}

#endif	


	//GetLocalTime(&systm);
	//fprintf(fpLog,"%2d:%2d:%3d 初始化结束。。。\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
	//fflush(fpLog);

	AMP_Display_InitDisplay(m_lDisplayPort,hwnd);

	m_yuvBuff[0] = new unsigned char[1920*1080*3/2];
	m_yuvBuff[1] = new unsigned char[1920*1080*3/2];
	InitializeCriticalSection(&m_csVideoYUV[0]);
	InitializeCriticalSection(&m_csVideoYUV[1]);
	m_iCurrentOutBuf = 0;

	m_audioBuff = new unsigned char[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	m_pAudioPlay = new AudioPlay();

	int iSampleSec = 48000;
	m_pAudioPlay->InitDSound(hwnd,iSampleSec);
	Set_tsDecoder_Volume(0);

	m_pAudioPlay->StartPlay();

	return true;
}


int TSDecoder_Instance::init_TS_Decoder(const char* cfilename,HWND hwnd,bool bflags,bool bSmooth,bool bNeedControlPlay,int iRTPServerPort,VideoCodeType vCodetype,AuidoCodeType aCodeType,const char* strDstIP,int iPort)
{
	strcpy(m_cfilename,cfilename);
	strcpy(m_strDstIP,strDstIP);
	m_iPort = iPort;
	printf("----filename=%s dstip=%s  port=%d \n",m_cfilename,m_strDstIP,m_iPort);
	m_vCodetype=vCodetype;
	m_aCodeType=aCodeType;
	m_bDecoderFlag = bflags;
	m_bSmoothPlay = bSmooth;
	m_bNeedControlPlay = bNeedControlPlay;


	pFFCodec = FFCodec::getInstance();
	pFFAVFormat = FFAvformat::getInstance();
	SYSTEMTIME systm;
	memset(&systm,0,sizeof(systm));
	GetLocalTime(&systm);
	fprintf(fpLog,"%2d:%2d:%3d 流地址：%s  初始化开始。。。\n",systm.wMinute,systm.wSecond,systm.wMilliseconds,m_cfilename);
	fflush(fpLog);

	//兼容rtsp
	//URLTYPE m_ifileType= tsUDP;
/*	char strURL[1024]={0};
	strcpy(strURL,m_cfilename);
	int port = 0;
	char cUDPPort[8] = {0};

	char pURL[1024] = {0};
	strcpy(pURL,strURL);
	char *pFind1 = strstr(pURL,"udp:");
	char *pFind2 = strstr(pURL,"rtsp:");
*/
	//  udp://@:14000 rtsp://192.168.20.131:8554/sd.ts
//	if(pFind1 != NULL)
	{
		//ts 流
		m_iURLType = tsUDP;
		//unsigned Decoder_thread;
		//	pthread_create(&Decoder_thread, NULL, TSDecoder_Instance::decoder_threadFun, this);
		//	pthread_detach(Decoder_thread);
		//启动数据回调线程
		HANDLE ThreadHandle;
		ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, decoder_threadFun, (LPVOID)this, 0, &m_iThreadid);
		CloseHandle (ThreadHandle);
	}
#if 0
	else if(pFind2 != NULL)
	{
		//rtsp 流
		m_iURLType = tsRTSP;

		char strRtspIP[256] ={0};
		//获取 ip 端口 资源
		char* pfindport1 = strstr(pURL,"//");
		if(pfindport1 != NULL)
		{

			char* pfindport2 = strstr(pfindport1,":");
			if(pfindport2)
			{
				char strPorttmp[1024] ={0};
				strcpy(strPorttmp,pfindport2);
				

				int ilen = pfindport2 - pfindport1;
				*(pfindport1+ilen) = '\0';
				strcpy(strRtspIP,pfindport1+2);
				char* pfingport3 = strstr(strPorttmp,"/");
				if(pfingport3)
				{
					*pfingport3 = '\0';

					port = atoi(strPorttmp+1);
				}
			}
		}
		//获取rtsp server的ip Port，
		memset(m_strRtspServerIP,0,sizeof(m_strRtspServerIP));
		strcpy(m_strRtspServerIP,strRtspIP);
		m_iRtspPort = port;

		//创建Myrtsp对象

		//开启rtsp连接线程

		//开启rtsp客户端

	}

#endif	


	//GetLocalTime(&systm);
	//fprintf(fpLog,"%2d:%2d:%3d 初始化结束。。。\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
	//fflush(fpLog);

	AMP_Display_InitDisplay(m_lDisplayPort,hwnd);

	m_yuvBuff[0] = new unsigned char[1920*1080*3/2];
	m_yuvBuff[1] = new unsigned char[1920*1080*3/2];
	InitializeCriticalSection(&m_csVideoYUV[0]);
	InitializeCriticalSection(&m_csVideoYUV[1]);
	m_iCurrentOutBuf = 0;

	m_audioBuff = new unsigned char[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	m_pAudioPlay = new AudioPlay();

	int iSampleSec = 48000;
	m_pAudioPlay->InitDSound(hwnd,iSampleSec);
	Set_tsDecoder_Volume(0);

	m_pAudioPlay->StartPlay();

	return true;
}

bool TSDecoder_Instance::initQueue(int isize)
{
	for(int i=0;i<isize;++i)
	{
		VideoData* tmpbuff = new VideoData;
		tmpbuff->data = new unsigned char[Video_Buff_size];
		m_freeQueue.push_back(tmpbuff);
	}
	
	//音频多点缓存
	for(int i=0;i<isize*5/2;++i)
	{
		AudioData* audiobuff = new AudioData;
		audiobuff->data = new unsigned char[Audio_Buff_size];
		m_audioFreeQueue.push_back(audiobuff);
	}
	
	
	return true;
}

bool TSDecoder_Instance::freeQueue()
{

	EnterCriticalSection(&m_locker);
	while(m_usedQueue.size() > 0)
	{
		VideoData* tmpbuff = m_usedQueue.front();
		m_usedQueue.pop_front();
		delete tmpbuff->data;
		tmpbuff->data = NULL;
		delete tmpbuff;
	}
	while(m_freeQueue.size() > 0)
	{
		VideoData* tmpbuff = m_freeQueue.front();
		m_freeQueue.pop_front();
		delete tmpbuff->data;
		delete tmpbuff;
		tmpbuff = NULL;
	}

	LeaveCriticalSection(&m_locker);

	EnterCriticalSection(&m_audioLocker);
	while(m_audioFreeQueue.size() > 0)
	{
		AudioData* audiobuff = m_audioFreeQueue.front();
		m_audioFreeQueue.pop_front();
		delete audiobuff->data;
		delete audiobuff;
		audiobuff = NULL;
	}
	while(m_audioUsedQueue.size() > 0)
	{
		AudioData* audiobuff = m_audioUsedQueue.front();
		m_audioUsedQueue.pop_front();
		delete audiobuff->data;
		delete audiobuff;
		audiobuff = NULL;
	}

	LeaveCriticalSection(&m_audioLocker);
	return true;
}


FILE* fpYUV = NULL;
void TSDecoder_Instance::Push_Video_Data(int iWidth, int iHight, AVFrame *pAVfram,unsigned long ulPTS)
{
	EnterCriticalSection(&m_locker);
	if(m_freeQueue.size() > 0)
	{
		VideoData* tempbuff = m_freeQueue.front();
		tempbuff->iHeight = iHight;
		tempbuff->iWidth = iWidth;
		tempbuff->pts = ulPTS;

		unsigned char *buff = tempbuff->data;
		for (int i = 0; i < iHight; i++)
		{
			memcpy(buff+i*iWidth, pAVfram->data[0]+i*pAVfram->linesize[0], iWidth);
		}
		for (int i=0; i < iHight/2; i++)
		{
			memcpy(buff+iHight*iWidth+i*iWidth/2, pAVfram->data[1]+i*pAVfram->linesize[1], iWidth/2);
		}
		for (int i=0; i < iHight/2; i++)
		{
			memcpy(buff+iHight*iWidth*5/4+i*iWidth/2, pAVfram->data[2]+i*pAVfram->linesize[2], iWidth/2);
		}
		/*
		if(fpYUV == NULL)
		{
			fpYUV = fopen("/home/ky/rsm-yyd/DecoderTs/jhxyuv.dat", "w+b");
		}
		fwrite(buff, 1, iWidth*iHight*3/2, fpYUV);
		*/
		m_freeQueue.pop_front();
		m_usedQueue.push_back(tempbuff);
	}
	else if(m_usedQueue.size()>0)
	{
		VideoData* tempbuff = m_usedQueue.front();
		tempbuff->iHeight = iHight;
		tempbuff->iWidth = iWidth;
		tempbuff->pts = ulPTS;

		unsigned char *buff = tempbuff->data;
		for (int i = 0; i < iHight; i++)
		{
			memcpy(buff+i*iWidth, pAVfram->data[0]+i*pAVfram->linesize[0], iWidth);
		}
		for (int i=0; i < iHight/2; i++)
		{
			memcpy(buff+iHight*iWidth+i*iWidth/2, pAVfram->data[1]+i*pAVfram->linesize[1], iWidth/2);
		}
		for (int i=0; i < iHight/2; i++)
		{
			memcpy(buff+iHight*iWidth*5/4+i*iWidth/2, pAVfram->data[2]+i*pAVfram->linesize[1], iWidth/2);
		}
		/*
		if(fpYUV == NULL)
		{
			fpYUV = fopen("/home/ky/rsm-yyd/DecoderTs/jhxyuv.dat", "w+b");
		}
		fwrite(buff, 1, iWidth*iHight*3/2, fpYUV);
		*/
		m_usedQueue.pop_front();
		m_usedQueue.push_back(tempbuff);
		printf("========full used front to back\n");
	}
	LeaveCriticalSection(&m_locker);
}

int TSDecoder_Instance::get_video_param(int *iwidth,int *iheight)
{
	*iwidth = m_iWidht;
	*iheight = m_iHeight;
	return true;
}

int TSDecoder_Instance::get_Video_data(unsigned char *output_video_yuv420,int *output_video_size,
	int* iwidth,int* iHeight,unsigned long* video_pts)
{
	//fprintf(stderr,"get video data\n");
/*	if(!m_bDecoderFlag && !m_recvqueue.m_bDelayFrame)
	{
		Clean_Video_audioQue();
		return -1;
	}*/
	EnterCriticalSection(&m_locker);
/*	if(!m_bDecoderFlag && m_recvqueue.m_bDelayFrame)
	{
		//printf("-----delay frame time 1111\n");
		if(m_usedQueue.size() > 0 )
		{
			m_iDelayFrame++;
			printf("-----delay frame time 2222\n");
			VideoData* tempbuff = m_usedQueue.front();
			
			unsigned char *buff = tempbuff->data;
			*iwidth = tempbuff->iWidth;
			*iHeight = tempbuff->iHeight;
			if(video_pts)
				*video_pts = tempbuff->pts;
			
			int iyuvsize = tempbuff->iWidth * tempbuff->iHeight*3/2;
			if(*output_video_size < iyuvsize)
			{
				fprintf(stderr,"output_video_size is too small \n");
				pthread_mutex_unlock(&m_locker);
				return -1;
			}
			*output_video_size = iyuvsize;
			memcpy(output_video_yuv420,buff,iyuvsize);
			tempbuff->iWidth = 0;
			tempbuff->iHeight = 0;
			m_usedQueue.pop_front();
			m_freeQueue.push_back(tempbuff);
			
			pthread_mutex_unlock(&m_locker);

			struct timeval tm;

			gettimeofday(&tm,NULL);
			printf("-----video que size =%d ,get Video Time =%ld\n",m_usedQueue.size(),tm.tv_sec*1000+tm.tv_usec/1000);
			//printf("pts=%ld,w=%d video queue used size =%d \n",m_ulvideo_pts,*iwidth,m_usedQueue.size());
			return 0;
		}
		else if(m_usedQueue.size() > 0)
		{
			m_iDelayFrame++;
			//m_recvqueue.m_bDelayFrame = false;
			printf("-----clean que begin\n");
			while(m_usedQueue.size() >0)
			{
				VideoData* tempbuff = m_usedQueue.front();
				tempbuff->iWidth = 0;
				tempbuff->iHeight = 0;
				m_usedQueue.pop_front();
				m_freeQueue.push_back(tempbuff);
			
			}
			pthread_mutex_unlock(&m_locker);
			printf("----clean video que \n");

			pthread_mutex_lock(&m_audioLocker);
			while(m_audioUsedQueue.size() > 0)
			{
				AudioData* audiobuff = m_audioUsedQueue.front();
				m_audioUsedQueue.pop_front();
				m_audioFreeQueue.push_back(audiobuff);
			}
			printf("----clean audio que \n");
			pthread_mutex_unlock(&m_audioLocker);
			
			return -1;
		}
		else
		{
			m_iDelayFrame++;
			pthread_mutex_unlock(&m_locker);
			return -1;
		}

	}
*/	
	if(m_usedQueue.size() >0)
	{
		VideoData* tempbuff = m_usedQueue.front();
		
		unsigned char *buff = tempbuff->data;
		*iwidth = tempbuff->iWidth;
		*iHeight = tempbuff->iHeight;
		if(video_pts)
			*video_pts = tempbuff->pts;
		//m_ulvideo_pts = tempbuff->pts; //控制音视频同步
		
		int iyuvsize = tempbuff->iWidth * tempbuff->iHeight*3/2;
		if(*output_video_size < iyuvsize)
		{
			printf("output_video_size is too small \n");
			LeaveCriticalSection(&m_locker);
			return -1;
		}
		*output_video_size = iyuvsize;
		memcpy(output_video_yuv420,buff,iyuvsize);
		tempbuff->iWidth = 0;
		tempbuff->iHeight = 0;
		m_usedQueue.pop_front();
		m_freeQueue.push_back(tempbuff);
		
		LeaveCriticalSection(&m_locker);



	}
	else
	{
		LeaveCriticalSection(&m_locker);
		return -1;
	}
	return 0;
}

void TSDecoder_Instance::Push_Audio_Data(unsigned char *sample,int isize,unsigned long ulPTS)
{
	EnterCriticalSection(&m_audioLocker);
	if(m_audioFreeQueue.size() > 0)
	{
		AudioData* audiobuff = m_audioFreeQueue.front();
		audiobuff->pts = ulPTS;
		audiobuff->size = isize;

		unsigned char *tmpbuf = audiobuff->data;
		memcpy(tmpbuf,sample,isize);

		m_audioFreeQueue.pop_front();
		m_audioUsedQueue.push_back(audiobuff);
	}
	else if(m_audioUsedQueue.size() > 0)
	{
		AudioData* audiobuff = m_audioUsedQueue.front();
		audiobuff->pts = ulPTS;
		audiobuff->size = isize;

		unsigned char* tmpbuf = audiobuff->data;
		memcpy(tmpbuf,sample,isize);

		m_audioUsedQueue.pop_front();
		m_audioUsedQueue.push_back(audiobuff);
	}
	
	LeaveCriticalSection(&m_audioLocker);
}

int TSDecoder_Instance::get_Audio_data(unsigned char *output_audio_data,int* input_audio_size,
	unsigned long* audio_pts)
{
	EnterCriticalSection(&m_audioLocker);
	if(m_audioUsedQueue.size() >0)
	{

		AudioData* tempbuff = m_audioUsedQueue.front();
		
		unsigned char *buff = tempbuff->data;
	
		if(audio_pts)
			*audio_pts= tempbuff->pts;
		
		if(*input_audio_size < tempbuff->size)
		{
			printf("input_audio_size is too small \n");
			LeaveCriticalSection(&m_audioLocker);
			return -1;
		}
		*input_audio_size= tempbuff->size;
		memcpy(output_audio_data,buff,tempbuff->size);
		tempbuff->size = 0;
		
		
		m_audioUsedQueue.pop_front();
		m_audioFreeQueue.push_back(tempbuff);
	
		LeaveCriticalSection(&m_audioLocker);
	}
	else
	{
		LeaveCriticalSection(&m_audioLocker);
		return -1;
	}
	return 0;
}

void TSDecoder_Instance::stopDecoder(bool bstop)
{
	m_bstop = bstop;
}

void av_free_packet(AVPacket *pkt)
{
    if (pkt) {
        if (pkt->destruct)
            pkt->destruct(pkt);
        pkt->data            = NULL;
        pkt->size            = 0;
        pkt->side_data       = NULL;
        pkt->side_data_elems = 0;
    }
}

void av_free(void *ptr)
{

    free(ptr);
}


void av_freep(void *arg)
{
    void **ptr= (void**)arg;
    av_free(*ptr);
    *ptr = NULL;
}


static void free_packet_buffer(AVPacketList **pkt_buf, AVPacketList **pkt_buf_end)
{
    while (*pkt_buf) {
        AVPacketList *pktl = *pkt_buf;
        *pkt_buf = pktl->next;
        av_free_packet(&pktl->pkt);
        av_freep(&pktl);
    }
    *pkt_buf_end = NULL;
}


bool TSDecoder_Instance::set_tsDecoder_stat(bool bstat)
{

	m_bDecoderFlag = bstat; //true 为自动匹配，FALSE为指定解码
	return true;

}

int TSDecoder_Instance::init_open_input()
{
	
	AVCodec *pVideoCodec, *pAudioCodec;
	AVCodecContext *pVideoCodecCtx = NULL;
	AVCodecContext *pAudioCodecCtx = NULL;
	AVIOContext * pb = NULL;
	AVInputFormat *piFmt = NULL;
	AVFormatContext *pFmt = NULL;


//	struct timeval tm;
			SYSTEMTIME systm;

			GetLocalTime(&systm);
			fprintf(fpLog,"%2d:%2d:%3d AutoMatch开始初始化ffmpeg\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);

	//av_register_all();
	//avformat_network_init();

	std::string strtmp=m_cfilename;
	std::size_t found = strtmp.find("\\");
	if (found == std::string::npos)
		m_ifileType = internetfile;
	else
	{
		m_ifileType = localfile;
	}
	printf("-------------file Type=%d\n",m_ifileType);

	if(m_ifileType == localfile || Use_ffmpeg_recv)
	{
		pFmt = pFFAVFormat->ff_avformat_alloc_context();
		if (pFFAVFormat->ff_avformat_open_input(&pFmt, m_cfilename, NULL, NULL) < 0)
		{
			printf( "avformat open failed.\n");
			return -1;
		} 
		else
		{
			printf("open stream success!\n");
		}
		
	}
	else if(m_ifileType == internetfile)
	{
		//  udp://@:14000
		std::size_t found  = strtmp.find("@:");
		int port = 0;
		if (found != std::string::npos)
		{
			std::string tm = strtmp.substr(found+2,(strtmp.length()-found-2));
			//printf("-------get string sub %s \n",tm.c_str());	
			port = atoi(tm.c_str());
			printf("=============get port =%d \n",port);
			
		}
		m_recvqueue.init_queue(Recv_Queue_size,port,m_strDstIP,m_iPort,fpLog,m_MediaInfofp,m_bNeedControlPlay);
		
		uint8_t *buf = (uint8_t*)pFFCodec->ff_av_mallocz(sizeof(uint8_t)*BUF_SIZE*2);
		//uint8_t *buf = (uint8_t*)malloc(sizeof(uint8_t)*BUF_SIZE);
		
		pb = pFFAVFormat->ff_avio_alloc_context(buf, BUF_SIZE, 0, this, TSDecoder_Instance::read_data, NULL, NULL);
		if (!pb) {
			printf( "avio alloc failed!\n");
			return -1;
		}

		
		// 开始探测码流
		//
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 开始探测码流类型\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
		//gettimeofday(&tm,NULL);
	//	printf("-----init time 1 =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
		int iloop = 5;
		do
		{
			if (pFFAVFormat->ff_av_probe_input_buffer(pb, &piFmt, "", NULL, 0, 188*7) < 0) {
				printf("probe failed!\n");
				Sleep(1);
			} else {
				printf("probe success!\n");
				//printf("format: %s[%s]\n", piFmt->name, piFmt->long_name);
				break;
			}
			
		}while(iloop-- >=0);


		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
	//	fprintf(fpLog,"%2d:%2d:%3d 探测码流成功\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
		//gettimeofday(&tm,NULL);
		//printf("-----init time 2 =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);

		pFmt = pFFAVFormat->ff_avformat_alloc_context();
		pFmt->pb = pb;
		if (pFFAVFormat->ff_avformat_open_input(&pFmt, "", piFmt, NULL) < 0) {
			//printf("avformat open failed.\n");
			memset(&systm,0,sizeof(systm));
			GetLocalTime(&systm);
			fprintf(fpLog,"%2d:%2d:%3d 打开网络数据失败\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);
			return -1;
		} else {
			memset(&systm,0,sizeof(systm));
			GetLocalTime(&systm);
			fprintf(fpLog,"%2d:%2d:%3d 打开网络数据成功\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);
		}

		
		//gettimeofday(&tm,NULL);
		//printf("-----init time 3 =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
		
	}


	//printf("======max_analyze_duration=%d,probesize=%ld==== \n",pFmt->max_analyze_duration,pFmt->probesize);

	if(m_ifileType == internetfile)
	{
		pFmt->max_analyze_duration  = 800;
		pFmt->probesize = BUF_SIZE;//2048;
	}



	if (pFFAVFormat->ff_av_find_stream_info(pFmt) < 0)
	{
		memset(&systm,0,sizeof(systm));
		GetLocalTime(&systm);
		fprintf(fpLog,"%2d:%2d:%3d 打开网络数据信息\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
		fflush(fpLog);
		return -1;
	}

	
	//gettimeofday(&tm,NULL);
	//printf("-----init time 4 =%ld\n",tm.tv_sec*1000+tm.tv_usec/1000);
	//pFFAVFormat->ff_av_dump_format(pFmt,0,"",0);
	



	//av_dump_format(pFmt, 0, "", 0);

	int videoindex = -1;
	int audioindex = -1;
	for (int i = 0; i < pFmt->nb_streams; i++) 
	{
		if ( (pFmt->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) &&
				(videoindex < 0) ) {
			videoindex = i;
		}
		if ( (pFmt->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) &&
				(audioindex < 0) ) {
			audioindex = i;
		}
	}


	if (videoindex < 0 || audioindex < 0) {
		printf("videoindex=%d, audioindex=%d\n", videoindex, audioindex);
		return -1;
	}

	AVStream *pVst,*pAst;
	pVst = pFmt->streams[videoindex];
	pAst = pFmt->streams[audioindex];

	
	pVideoCodecCtx = pVst->codec;
	pAudioCodecCtx = pAst->codec;

	pVideoCodec = pFFCodec->ff_avcodec_find_decoder(pVideoCodecCtx->codec_id);
	if (!pVideoCodec) {
		printf("could not find video decoder!\n");
		return -1;
	}
	if (pFFCodec->ff_avcodec_open(pVideoCodecCtx, pVideoCodec) < 0) {
		printf("could not open video codec!\n");
		return -1;
	}


	pAudioCodec = pFFCodec->ff_avcodec_find_decoder(pAudioCodecCtx->codec_id);
	if (!pAudioCodec) {
		printf("could not find audio decoder!\n");
		return -1;
	}
	if (pFFCodec->ff_avcodec_open(pAudioCodecCtx, pAudioCodec) < 0) {
		printf("could not open audio codec!\n");
		return -1;
	}
	

	memset(&systm,0,sizeof(systm));
	GetLocalTime(&systm);


	// media info
	char txt[1024]={0};
	switch(pVideoCodecCtx->codec_id)
	{
		case CODEC_ID_H264:
			{
				memset(txt,0,sizeof(txt));
				sprintf(txt,"Video Format H264\n");
				fprintf(fpLog,"%2d:%2d:%3d 视频类型：H264\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
				fflush(fpLog);
				break;
			}
		case CODEC_ID_MPEG2VIDEO:
			{
				memset(txt,0,sizeof(txt));
				sprintf(txt,"Video Format MPEG2VIDEO\n");
				fprintf(fpLog,"%2d:%2d:%3d 视频类型：MPEG2VIDEO\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
				fflush(fpLog);
				break;
			}
		case CODEC_ID_MPEG4:
			{
				memset(txt,0,sizeof(txt));
				sprintf(txt,"Video Format MPEG4\n");
				fprintf(fpLog,"%2d:%2d:%3d 视频类型：MPEG4\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
				fflush(fpLog);
				break;
			}
		case CODEC_ID_H263:
			{
				memset(txt,0,sizeof(txt));
				sprintf(txt,"Video Format H263\n");
				fprintf(fpLog,"%2d:%2d:%3d 视频类型：H263\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
				fflush(fpLog);
				break;
			}
		default:
			{
				memset(txt,0,sizeof(txt));
				sprintf(txt,"Video Format %d\n",pVideoCodecCtx->codec_id);
				fprintf(fpLog,"%2d:%2d:%3d 视频类型：%d\n",systm.wMinute,systm.wSecond,systm.wMilliseconds,pVideoCodecCtx->codec_id);
				fflush(fpLog);
			}
			break;
	}
	fwrite(txt,1,strlen(txt),m_MediaInfofp);
	fflush(m_MediaInfofp);

	switch(pAudioCodecCtx->codec_id)
	{
	case CODEC_ID_MP2:
		{
			memset(txt,0,sizeof(txt));
			sprintf(txt,"Audio Format MP2\n");
			fprintf(fpLog,"%2d:%2d:%3d 音频类型：MP2\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);
			break;
		}
	case CODEC_ID_MP3:
		{
			memset(txt,0,sizeof(txt));
			sprintf(txt,"Audio Format MP3\n");
			fprintf(fpLog,"%2d:%2d:%3d 音频类型：MP3\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);
			break;
		}
	case CODEC_ID_AAC:
		{
			memset(txt,0,sizeof(txt));
			sprintf(txt,"Audio Format AAC\n");
			fprintf(fpLog,"%2d:%2d:%3d 音频类型：AAC\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);
			break;
		}
	case CODEC_ID_AC3:
		{
			memset(txt,0,sizeof(txt));
			sprintf(txt,"Audio Format AC3\n");
			fprintf(fpLog,"%2d:%2d:%3d 音频类型：AC3\n",systm.wMinute,systm.wSecond,systm.wMilliseconds);
			fflush(fpLog);
			break;
		}
	default:
		{
			memset(txt,0,sizeof(txt));
			sprintf(txt,"Audio Format %d\n",pAudioCodecCtx->codec_id);
			fprintf(fpLog,"%2d:%2d:%3d 音频类型：%d\n",systm.wMinute,systm.wSecond,systm.wMilliseconds,pAudioCodecCtx->codec_id);
			fflush(fpLog);
		}
		break;
	}
	fwrite(txt,1,strlen(txt),m_MediaInfofp);
	fflush(m_MediaInfofp);




	m_pVideoCodec = pVideoCodec;
	m_pAudioCodec = pAudioCodec;
	m_pFmt = pFmt;
	m_pVideoCodecCtx = pVideoCodecCtx;
	m_pAudioCodecCtx = pAudioCodecCtx;
	m_pframe = NULL;
	//m_iHeight = iHeight;
	//m_iWidht = iWidht;
	m_videoindex = videoindex;
	m_audioindex = audioindex;

	
//	pthread_mutex_init(&m_locker,NULL);
//	pthread_mutex_init(&m_audioLocker,NULL);
	InitializeCriticalSection(&m_audioLocker);
	InitializeCriticalSection(&m_locker);
	
	//initQueue(Queue_size);

	return 0;
}

bool TSDecoder_Instance::Get_tsDecoder_sem(void **pSem)
{
	//*pSem = &(m_recvqueue.m_sem_send);
	printf("=====printf add \n");
//	printf("----sem add %0x",*pSem);
	return true;
}



int TSDecoder_Instance::uninit_TS_Decoder()
{
	
	//pthread_cancel(p_instanse->read_thread_id);
	//pthread_cancel(p_instanse->write_thread_id);
	//pthread_mutex_destroy(&p_instanse->m_mutex);

	freeQueue();
	
	if (m_pVideoCodecCtx) 
	{
        pFFCodec->ff_avcodec_close(m_pVideoCodecCtx);
    }
	if(m_pAudioCodecCtx)
	{
		pFFCodec->ff_avcodec_close(m_pAudioCodecCtx);
	}
	//if(p_instanse->m_pFmt)
	//{
	//	avcodec_close(p_instanse->m_pFmt);
	//}
	pFFCodec->ff_av_freep(m_pframe);

    /* free the stream */
    pFFCodec->ff_av_freep(m_pFmt);

//	if(m_avbuf)
//		pFFCodec->ff_av_freep(m_avbuf);
//	m_avbuf = NULL;
	return 0;
}

bool TSDecoder_Instance::Clean_Video_audioQue()
{
	EnterCriticalSection(&m_locker);
	//m_recvqueue.m_bDelayFrame = false;
	//printf("-----clean video que begin\n");
	while(m_usedQueue.size() >0)
	{
		VideoData* tempbuff = m_usedQueue.front();
		tempbuff->iWidth = 0;
		tempbuff->iHeight = 0;
		m_usedQueue.pop_front();
		m_freeQueue.push_back(tempbuff);

	}
	LeaveCriticalSection(&m_locker);
	//printf("----clean audio que \n");

	EnterCriticalSection(&m_audioLocker);
	while(m_audioUsedQueue.size() > 0)
	{
		AudioData* audiobuff = m_audioUsedQueue.front();
		m_audioUsedQueue.pop_front();
		m_audioFreeQueue.push_back(audiobuff);
	}
	LeaveCriticalSection(&m_audioLocker);

	return true;
}

bool TSDecoder_Instance::Set_tsDecoder_Volume(int iVolume)
{
	double fbtmp = (double)iVolume/100;
	DWORD nVolume = fbtmp * 0xFFFF;
	if(!m_pAudioPlay)
		return false;
	return m_pAudioPlay->SetVolume(nVolume);
}

bool TSDecoder_Instance::Get_tsDecoder_Volume(int &iVolume)
{
	if(!m_pAudioPlay)
		return false;

	DWORD nVolume = 0;
	int ret = true;
	if(m_pAudioPlay)
		m_pAudioPlay->GetVolume(nVolume);
	double fbtmp = (double)nVolume /0xFFFF;
	 iVolume = fbtmp *100;
	return ret;
}

//设置码率周期
bool TSDecoder_Instance::Set_tsRate_period(int iperiod)
{

	m_recvqueue.Set_tsRate_period(iperiod);
	return true;
}

//获取到码率
bool TSDecoder_Instance::Get_tsRate(int* iRate)
{
	m_recvqueue.Get_tsRate(iRate);
	return true;
}
//计算延时
bool TSDecoder_Instance::Set_tsTime_delay(int begintime,int* relsutTime)
{

	return true;
}

bool TSDecoder_Instance::Get_tsIFrame_size(int* iSize)
{
	m_tsStreamPrase.Get_tsIFrame_size(iSize);
//	m_recvqueue.Get_tsIFrame_size(iSize);
	return true;
}

bool TSDecoder_Instance::Set_tsDecoder_SaveStream(bool bSaveStream)
{
	//m_tsStreamPrase.Set_tsDecoder_SaveStream(bSaveStream);
	m_bSaveTSStream = bSaveStream;
	return true;
}