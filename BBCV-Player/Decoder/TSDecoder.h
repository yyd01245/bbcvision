/*
	author yyd
		
*/

#ifndef __TSDECODER_H__
#define __TSDECODER_H__

#include <Windows.h>
typedef void TSDecoder_t;

typedef enum enVideoType
{
	CODE_HD_VIDEO = 1,
	CODE_SD_VIDEO
}VideoCodeType;

typedef enum enAuidoCodeType
{
	CODE_AUIDO_MP2=1,
	CODE_AUIDO_MP3,
	CODE_AUDIO_AAC
}AuidoCodeType;


typedef enum
{
	tsUDP =1,
	tsRTSP = 2,
	tsLocalFile =3,
	othertype
}URLTYPE;

typedef struct _DecodeParam
{
	bool bNeedLog;				//需要日志
	bool bNeedControlPlay;		//控制播放
	bool bSmooth;				//平滑播放
	bool bAutoMatch;			//自动匹配
	bool bSaveVideoData;		//保存视频数据
	bool bDelayCheckModel;		//延时检测机制
	VideoCodeType vCodetype;    //视频类型
	AuidoCodeType aCodetype;	//音频类型
	char strDstIP[128];			//目的ip
	int iport;					//目的端口

}DecoderControlParam;

//<summary>
//<param cfilename> 文件名
//<param iWidht>  指定输出YUV的widht
//<param iHight> 指定输出YUV的height
TSDecoder_t* init_TS_Decoder(const char* cfilename,HWND hwnd,DecoderControlParam dcParam);

//<summary>
//<param cfilename> 文件名
//<param iWidht>  指定输出YUV的widht
//<param iHight> 指定输出YUV的height
TSDecoder_t* init_TS_Decoder(const char* cfilename,HWND hwnd, bool bflags,bool bSmooth,bool bNeedControlPlay=false, int iRTPServerPort=0,VideoCodeType vCodetype=CODE_HD_VIDEO,AuidoCodeType aCodeType=CODE_AUIDO_MP2,
									const char* strDstIP="192.168.60.246",int iPort=10000);

//<summary>
//<param ts_decoder>  instance
//<param pWidth pHeight> OUT 输出YUV的宽和高
int get_Video_Param(TSDecoder_t *ts_decoder,int *pWidth,int *pHeight); 


//<summary>
//<param ts_decoder>  instance
//<param output_video_yuv420>  OUT 输出YUV420数据
//<param output_video_size> OUT IN 输入output_video_yuv420的容量大小， 输出output_video_yuv420实际长度
//<param pWidth pHeight> OUT 输出YUV的宽和高
//<param video_pts> OUT video PTS

int get_Video_data(TSDecoder_t *ts_decoder,unsigned char *output_video_yuv420,int *output_video_size,
	int *pWidth,int *pHeight,unsigned long *video_pts=0); 

//<summary>
//<param ts_decoder>  instance
//<param output_audio_data>  OUT 输出音频数据
//<param output_audio_size> OUT IN 输入output_video_data的容量大小， 输出output_audio_data实际长度
//<param audio_pts> OUT 输出音频数据的PTS

int get_Audio_data(TSDecoder_t *ts_decoder,unsigned char *output_audio_data,int* input_audio_size,
	unsigned long* audio_pts);

//<summary> 设置解码开关，true 为自动匹配，FALSE为指定解码
bool Set_tsDecoder_stat(TSDecoder_t *ts_decoder,bool bStart);

//设置码率周期
bool Set_tsRate_period(TSDecoder_t *ts_decoder,int iperiod);

//获取到码率
bool Get_tsRate(TSDecoder_t *ts_decoder,int* iRate);
//计算延时
bool Set_tsTime_delay(TSDecoder_t *ts_decoder,int begintime,int* relsutTime);

bool Get_tsIFrame_size(TSDecoder_t *ts_decoder,int* iSize);


bool Get_tsDecoder_sem(TSDecoder_t *ts_decoder,void **pSem);

bool Set_tsDecoder_Volume(TSDecoder_t *ts_decoder,int iVolume);

bool Get_tsDecoder_Volume(TSDecoder_t *ts_decoder,int &iVolume);

bool Set_tsDecoder_SaveStream(TSDecoder_t *ts_decoder,int bsave);

int uninit_TS_Decoder(TSDecoder_t *audioencoder);

#endif

