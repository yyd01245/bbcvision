/*
	author yyd
		
*/

#ifndef __DECODER_H_
#define __DECODER_H_

#include <deque>
#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include <Windows.h>
//#include <sys/types.h>
//#include <unistd.h>
#include "..\AMPDisplaySDK\AMPDisplaySDK.h"
//#include <pthread.h>
#include <process.h>
#include "TSDecoder.h"
#include "AudioPlay.h"

#include "..\FFCodec\FFCodec.h"
#include "..\FFCodec\FFAvformat.h"
#include "TSStreamInfo.h"

/*
#ifndef INT64_C
#define INT64_C(c) c##LL
#endif
#ifndef UINT64_C
#define UINT64_C(c) c##LL
#endif
*/
/*
extern "C" {

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

}
*/
//#include <string.h>
//#include <string>
#include "RecvQueue.h"

/*
class FFCodec;
class FFAvformat;
//
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
//
struct AVFormatContext;
struct AVPacket;

struct AVIOContext;
//struct AVInputFormat;
*/
typedef struct _VideoData {
	unsigned char *data;
	long size;
	unsigned long pts;
	int iWidth;
	int iHeight;
} VideoData;

typedef enum
{
	internetfile =1,
	localfile = 2,
	other =3
}TSFILETYPE;



typedef struct _AudioData {
	unsigned char *data;
	long size;
	unsigned long pts;
} AudioData;





class TSDecoder_Instance
{
public:
	TSDecoder_Instance();
	~TSDecoder_Instance();
	
	int init_TS_Decoder(const char* cfilename,HWND hwnd,DecoderControlParam dcParam);


	int init_TS_Decoder(const char* cfilename,HWND hwnd,bool bflags,bool bSmooth,bool bNeedControlPlay=false,int iRTPServerPort=0,VideoCodeType vCodetype=CODE_HD_VIDEO,AuidoCodeType aCodeType=CODE_AUIDO_MP2,const char* strDstIP="192.168.60.246",int iPort=10000);
	
	int get_Video_data(unsigned char *output_video_yuv420,int *output_video_size,
	int* iwidth,int* iHeight,unsigned long* video_pts=NULL); 

	void Push_Video_Data(int iWidth, int iHight, AVFrame *pAVfram,unsigned long ulPTS);
	
	int get_Audio_data(unsigned char *output_audio_data,int* input_audio_size,
	unsigned long* audio_pts);

	void Push_Audio_Data(unsigned char *sample,int isize,unsigned long ulPTS);

	int uninit_TS_Decoder();

	static unsigned int _stdcall decoder_threadFun(void *param);

	void stopDecoder(bool bstop);

	bool initQueue(int isize);

	bool freeQueue();

	int get_video_param(int *iwidth,int *iheight);

	int initdecoder();

	bool set_tsDecoder_stat(bool bstat);
	
	static int read_data(void *opaque, uint8_t *buf, int buf_size);

	int get_queue(uint8_t* buf, int size);

	bool open_inputdata();

	int init_open_input();

	bool Clean_Video_audioQue();
	bool Get_tsDecoder_sem(void **pSem);
	bool Set_tsDecoder_Volume(int iVolume);
	bool Get_tsDecoder_Volume(int &iVolume);

	//设置码率周期
	bool Set_tsRate_period(int iperiod);

	//获取到码率
	bool Get_tsRate(int* iRate);
	//计算延时
	bool Set_tsTime_delay(int begintime,int* relsutTime);

	bool Get_tsIFrame_size(int* iSize);

	bool Set_tsDecoder_SaveStream(bool bSaveStream);
private:

	URLTYPE m_iURLType;

	char m_strRtspServerIP[256];
	int m_iRtspPort;

	FFCodec *pFFCodec;			//解码对象
	FFAvformat *pFFAVFormat;	//码流分析对象
	LONG m_lDisplayPort;
	unsigned char *m_yuvBuff[2];
	int m_iCurrentOutBuf;
	CRITICAL_SECTION m_csVideoYUV[2];

	uint8_t *m_avbuf;

	FILE *fpLog ;

	FILE *m_MediaInfofp ;
		char m_meidaInfoPath[256];

	bool m_bNeedControlPlay;


	unsigned char *m_audioBuff;

	AVCodec *m_pVideoCodec;
	AVCodec *m_pAudioCodec;
	AVCodecContext *m_pVideoCodecCtx;
	AVCodecContext *m_pAudioCodecCtx;
	AVIOContext * m_pb;
	//AVInputFormat *m_piFmt;
	AVFormatContext *m_pFmt;
	AVFrame *m_pframe;
	int m_iWidht;
	int m_iHeight;
	int m_videoindex;
	int m_audioindex;
	std::deque<VideoData*> m_usedQueue;
	std::deque<VideoData*> m_freeQueue;	
	CRITICAL_SECTION m_locker;
	//CRITICAL_SECTION m_locker;

	AudioPlay * m_pAudioPlay;

	std::deque<AudioData*> m_audioUsedQueue;
	std::deque<AudioData*> m_audioFreeQueue;
	CRITICAL_SECTION m_audioLocker;

	unsigned long m_ulvideo_pts;

	unsigned m_iThreadid;
	bool m_bstop;
	TSFILETYPE m_ifileType;//url 1 or file 2

	NewQueue m_recvqueue;
	char m_cfilename[1024];

	char m_strDstIP[256];
	int m_iPort;

	VideoCodeType m_vCodetype;
	AuidoCodeType m_aCodeType;
	bool m_bDecoderFlag;//true decoder Auto,
	bool m_bFirstDecodeSuccess; //I frame decode

	TSstreamInfo m_tsStreamPrase;//解析ts

	bool m_bSmoothPlay;
	int m_iDelayFrame;
	bool m_bSaveTSStream; //保存ts流
	FILE *m_tsStreamfp;


	int m_iperiod;//码率周期
	int m_iIFrameSize;//上一个Iframe大小

};


#endif
