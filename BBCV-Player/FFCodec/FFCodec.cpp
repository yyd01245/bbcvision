// FFCodec.cpp: implementation of the FFCodec class.
//
//////////////////////////////////////////////////////////////////////
#include <atlbase.h>
#include <string>
#include "FFCodec.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
FFCodec* FFCodec::theInstance = 0;


FFCodec::FFCodec()
{ 

	DWORD err = 0; 

	char path[200] = {0};
	char temp[200] = {0};
	char strdll[20] = "avcodec-54.dll";

	//获取当前dll路径
	GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase),temp,200);
	int i = 0;
	for(i = strlen(temp); temp[i-1]!='\\';i--);
	for (int j = 0; j <= strlen(strdll); ++j, ++i)
	{
		temp[i] = strdll[j];
	}
	temp[i+1] = '\0';
	int it = 0;
	for (int j=0; j<=strlen(temp); ++j, ++it)
	{
		path[it] = temp[j];
		if (temp[j] == '\\')
		{
			path[++it] = '\\';
		}
	}
	path[++it] = '\0';
	//获取路径结束


	avcodecDLLInstance = NULL;
	//avcodecDLLInstance =LoadLibrary("avcodec-54.dll");
	avcodecDLLInstance =LoadLibrary(path);

	memset(path, 0, 200);
	memset(temp, 0, 200);
	char strdll1[20] = "avutil-51.dll";

	//获取当前dll路径
	GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase),temp,200);
	i = 0;
	for(i = strlen(temp); temp[i-1]!='\\';i--);
	for (int j = 0; j <= strlen(strdll1); ++j, ++i)
	{
		temp[i] = strdll1[j];
	}
	temp[i+1] = '\0';
	it = 0;
	for (int j=0; j<=strlen(temp); ++j, ++it)
	{
		path[it] = temp[j];
		if (temp[j] == '\\')
		{
			path[++it] = '\\';
		}
	}
	path[++it] = '\0';
	//获取路径结束

	avutilDLLInstance = NULL;
	//avutilDLLInstance = LoadLibrary("avutil-52.dll");
	avutilDLLInstance = LoadLibrary(path);

	if(avcodecDLLInstance != NULL && avutilDLLInstance != NULL)
	{
		//ffavcodec_init = NULL;
		//ffavcodec_init = (_FFAVCODEC_INIT)GetProcAddress(dllInstance,"avcodec_init");
	
		ffavcodec_register_all = NULL;
		ffavcodec_register_all = (_FFAVCODEC_REGISTER_ALL)GetProcAddress(avcodecDLLInstance,"avcodec_register_all");

		ffavcodec_find_decoder = NULL;
		ffavcodec_find_decoder = (_FFAVCODEC_FIND_DECODER)GetProcAddress(avcodecDLLInstance,"avcodec_find_decoder");

		ffavcodec_find_encoder = NULL;
		ffavcodec_find_encoder = (_FFAVCODEC_FIND_ENCODER)GetProcAddress(avcodecDLLInstance,"avcodec_find_encoder");
	
		ffavcodec_alloc_context = NULL;
		ffavcodec_alloc_context = (_FFAVCODEC_ALLOC_CONTEXT)GetProcAddress(avcodecDLLInstance,"avcodec_alloc_context");

		ffavcodec_alloc_frame = NULL;
		ffavcodec_alloc_frame = (_FFAVCODEC_ALLOC_FRAME)GetProcAddress(avcodecDLLInstance,"avcodec_alloc_frame");

		ffavcodec_open = NULL;
		ffavcodec_open = (_FFAVCODEC_OPEN)GetProcAddress(avcodecDLLInstance,"avcodec_open");
		
		ffavcodec_decode_video = NULL;
		ffavcodec_decode_video = (_FFAVCODEC_DECODE_VIDEO)GetProcAddress(avcodecDLLInstance,"avcodec_decode_video2");

		ffavcodec_encode_video = NULL;
		ffavcodec_encode_video = (_FFAVCODEC_ENCODE_VIDEO)GetProcAddress(avcodecDLLInstance,"avcodec_encode_video");

		ffavcodec_close = NULL;
		ffavcodec_close = (_FFAVCODEC_CLOSE)GetProcAddress(avcodecDLLInstance,"avcodec_close");
		
		ffav_freep = NULL;
		ffav_freep = (_FFAV_FREEP)GetProcAddress(avutilDLLInstance,"av_free");

		ffavpicture_deinterlace = NULL;
		ffavpicture_deinterlace = (_FFAVPICTURE_DEINTERLACE)GetProcAddress(avcodecDLLInstance,"avpicture_deinterlace");

		ffavpicture_alloc = NULL;
		ffavpicture_alloc = (_FFAVPICTURE_ALLOC)GetProcAddress(avcodecDLLInstance,"avpicture_alloc");

		ffav_init_packet = NULL;
		ffav_init_packet = (_FFAV_INIT_PACKET)GetProcAddress(avcodecDLLInstance,"av_init_packet");

		ffavcodec_free_frame = NULL;
		ffavcodec_free_frame = (_FFAVCODEC_FREE_FRAME)GetProcAddress(avcodecDLLInstance,"avcodec_free_frame");

		ffavcodec_flush_buffers = NULL;
		ffavcodec_flush_buffers = (_FFAVCODEC_FLUSH_BUFFERS)GetProcAddress(avcodecDLLInstance,"avcodec_flush_buffers");

		ffav_free_packet = NULL;
		ffav_free_packet = (_FFAV_FREE_PACKET)GetProcAddress(avcodecDLLInstance,"av_free_packet");

		ffav_mallocz= NULL;
		ffav_mallocz = (_FFAV_AV_MALLOCZ)GetProcAddress(avutilDLLInstance,"av_mallocz");

		ffavcodec_decode_audio = NULL;
		ffavcodec_decode_audio = (_FFAVCODEC_DECODE_AUDIO)GetProcAddress(avcodecDLLInstance,"avcodec_decode_audio3");

		ffavcodec_decode_audio4 = NULL;
		ffavcodec_decode_audio4 = (_FFAVCODEC_DECODE_AUDIO4)GetProcAddress(avcodecDLLInstance,"avcodec_decode_audio4");
		//ff_avcodec_init();
		ff_avcodec_register_all();
	}
	else
	{
	   err = GetLastError();
	   char tmp[64];
	   sprintf(tmp,"LoadLibrary avcodec-54.dll fail err %d\n",err);
	   OutputDebugString(tmp);
	}
}

FFCodec::~FFCodec()
{
#ifdef use_dll

	if(avcodecDLLInstance)
	{
		FreeLibrary(avcodecDLLInstance);
		avcodecDLLInstance = NULL;
	}
#endif 

}


//void FFCodec::ff_avcodec_init()
//{
//	if(ffavcodec_init)
//		ffavcodec_init();
//}
void FFCodec::ff_avcodec_register_all()
{
	if(ffavcodec_register_all)
		ffavcodec_register_all();
}


AVCodec *FFCodec::ff_avcodec_find_decoder(enum CodecID id)
{
	if(ffavcodec_find_decoder)
		return (AVCodec *)ffavcodec_find_decoder(id);
	else
		return NULL;
}
AVCodec *FFCodec::ff_avcodec_find_encoder(enum CodecID id)
{
	if(ffavcodec_find_decoder)
		return (AVCodec *)ffavcodec_find_encoder(id);
	else
		return NULL;
}

AVCodecContext *FFCodec::ff_avcodec_alloc_context()
{
	if(ffavcodec_alloc_context)
		return (AVCodecContext *)ffavcodec_alloc_context();
	else 
		return NULL;
}
AVFrame *FFCodec::ff_avcodec_alloc_frame()
{
	if(ffavcodec_alloc_frame)
		return (AVFrame *)ffavcodec_alloc_frame();
	else 
		return NULL;
}
int FFCodec::ff_avcodec_open(AVCodecContext *context,AVCodec *code)
{
	if(ffavcodec_open)
		return ffavcodec_open(context,code);
	else
		return -1;
}
int FFCodec::ff_avcodec_decode_video(AVCodecContext *context,AVFrame *frame,int *got_picture_ptr,const AVPacket *avpkt)
{
	if(ffavcodec_decode_video)
	 
		return ffavcodec_decode_video(context,frame,got_picture_ptr,avpkt);   
	else 
		return -1;
}
int FFCodec::ff_avcodec_encode_video(AVCodecContext *context,unsigned char *buf, int buf_size, const AVFrame *pict)
{
	if(ffavcodec_encode_video)
		return ffavcodec_encode_video(context,buf,buf_size,pict);
	else 
		return -1;
}

int FFCodec::ff_avcodec_close(AVCodecContext *context)
{
	if(ffavcodec_close)
		return ffavcodec_close(context);
	else 
		return -1;
}

void FFCodec::ff_av_freep(void *ptr)
{
	if(ffav_freep)
		 ffav_freep(ptr);
	else 
		return ;
}

int FFCodec::ff_avpicture_deinterlace(AVPicture *dst, const AVPicture *src, int pix_fmt, int width, int height)
{
	if(ffavpicture_deinterlace)
		return ffavpicture_deinterlace(dst, src, pix_fmt, width, height);
	else 
		return -1;
}

int FFCodec::ff_avpicture_alloc(AVPicture *picture, int pix_fmt, int width, int height)
{
	if(ffavpicture_alloc)
		return ffavpicture_alloc(picture, pix_fmt, width, height);
	else 
		return -1;
}

void FFCodec::ff_av_init_packet(AVPacket *pkt)
{
	if(ffav_init_packet)
		ffav_init_packet(pkt);
	else 
		return;
}

void FFCodec::ff_avcodec_free_frame(AVFrame **frame)
{
	if(ffavcodec_free_frame)
		ffavcodec_free_frame(frame);
	else 
		return;
}
void FFCodec::ff_avcodec_flush_buffers(AVCodecContext *avctx)
{
	if(ffavcodec_flush_buffers)
		ffavcodec_flush_buffers(avctx);
}

void FFCodec::ff_av_free_packet(AVPacket *pkt)
{
	if(ffav_free_packet)
		ffav_free_packet(pkt);
	else 
		return;
}

void* FFCodec::ff_av_mallocz(size_t size)
{
	if(ffav_mallocz)
		return (void*)ffav_mallocz(size);
	else 
		return NULL;
}


int FFCodec::ff_avcodec_decode_audio(AVCodecContext *avctx, int16_t *samples,
							int *frame_size_ptr,
							AVPacket *avpkt)
{
	if(ffavcodec_decode_audio)
		return ffavcodec_decode_audio(avctx,samples,frame_size_ptr,avpkt);
	else 
	{
		printf("------\n");
		return -1;
	}
}

int FFCodec::ff_avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
							 int *got_frame_ptr, const AVPacket *avpkt)
{
	if(ffavcodec_decode_audio4)
		return ffavcodec_decode_audio4(avctx,frame,got_frame_ptr,avpkt);
	else 
	{
		printf("------\n");
		return -1;
	}
}
