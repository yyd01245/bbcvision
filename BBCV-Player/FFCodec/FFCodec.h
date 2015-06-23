// FFCodec.h: interface for the FFCodec class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FFCODEC_H__8981C6ED_A189_4107_9B61_075B506B3A11__INCLUDED_)
#define AFX_FFCODEC_H__8981C6ED_A189_4107_9B61_075B506B3A11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef INT64_C
#define INT64_C(c) c##LL
#endif
#ifndef UINT64_C
#define UINT64_C(c) c##LL
#endif

#define  use_dll  //!!!!!!!!!!!!!!!!!!!!!!!!here

#ifdef use_dll

#include ".\libavcodec\avcodec.h"

#else 


#ifdef __cplusplus
extern "C" {
#endif

#pragma comment(lib, "libgcc.a")
#pragma comment(lib, "libavcodec.a")
//#pragma comment(lib, "libavformat.a")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libavcore.a")
#pragma comment(lib, "libmingwex.a")
#pragma comment(lib, "libpthread.dll.a")



#define INT8_C(val) val
#define UINT8_C(val) val
#define INT16_C(val) val
#define UINT16_C(val) val

#define INT32_C(val) val
#define UINT32_C(val) val##U
#define INT64_C(val) val##LL
#define UINT64_C(val) val##ULL


#include "./ffmpeg-lib/include/libavcodec/avcodec.h"

#ifdef __cplusplus
}
#endif

#endif 



#include "windows.h"

////void avcodec_init(void);
//typedef (_cdecl *_FFAVCODEC_INIT) ();

//void avcodec_register_all(void);
typedef (_cdecl *_FFAVCODEC_REGISTER_ALL) ();

//AVCodec *avcodec_find_decoder(enum CodecID id);
typedef /*(AVCodec *)*/(_cdecl *_FFAVCODEC_FIND_DECODER)    (int );

//AVCodec *avcodec_find_encoder(enum CodecID id);
typedef /*(AVCodec *)*/(_cdecl *_FFAVCODEC_FIND_ENCODER)    (int );

//AVCodecContext *avcodec_alloc_context(void);
typedef /*(AVCodecContext *)*/(_cdecl *_FFAVCODEC_ALLOC_CONTEXT)    ();

//AVFrame *avcodec_alloc_frame(void);
typedef /*(AVFrame *)*/(_cdecl *_FFAVCODEC_ALLOC_FRAME) ();

//avcodec_get_context_defaults
typedef (_cdecl *_FFAVCODEC_GET_CONTEXT_DEFAULTS) (AVCodecContext *);

//int avcodec_open(AVCodecContext *avctx, AVCodec *codec);
typedef int (_cdecl *_FFAVCODEC_OPEN) (AVCodecContext *,AVCodec *);

//int avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, const AVPacket *avpkt);
typedef int (_cdecl *_FFAVCODEC_DECODE_VIDEO) (AVCodecContext *,AVFrame *,int *,const AVPacket *);

//int avcodec_encode_video(AVCodecContext *avctx, uint8_t *buf, int buf_size, const AVFrame *pict);
typedef int (_cdecl *_FFAVCODEC_ENCODE_VIDEO) (AVCodecContext *, unsigned char *, int , const AVFrame *);

//int avcodec_close(AVCodecContext *avctx);
typedef int (_cdecl *_FFAVCODEC_CLOSE) (AVCodecContext *);

typedef void (_cdecl *_FFAV_FREEP) (void *);

//int avcodec_close(AVCodecContext *avctx);
typedef int (_cdecl *_FFAVPICTURE_DEINTERLACE) (AVPicture *dst, const AVPicture *src, int pix_fmt, int width, int height);

//int avpicture_alloc(AVPicture *picture, int pix_fmt, int width, int height);
typedef int (_cdecl *_FFAVPICTURE_ALLOC) (AVPicture *picture, int pix_fmt, int width, int height);

//void av_init_packet(AVPacket *pkt);
typedef void (_cdecl *_FFAV_INIT_PACKET) (AVPacket *);

//void avcodec_free_frame(AVFrame **frame);
typedef void (_cdecl *_FFAVCODEC_FREE_FRAME) (AVFrame **);

//// void av_free_packet(AVPacket *pkt);
//typedef void (_cdecl *_FFAV_FREE_PACKET)(AVPacket *pkt);

//void avcodec_flush_buffers(AVCodecContext *avctx);
typedef void (_cdecl *_FFAVCODEC_FLUSH_BUFFERS)(AVCodecContext *avctx);

//void av_free_packet(AVPacket *pkt);
typedef void (_cdecl *_FFAV_FREE_PACKET)(AVPacket *pkt);

//void *av_mallocz(size_t size)
typedef (_cdecl *_FFAV_AV_MALLOCZ)(size_t size);

//int avcodec_decode_audio3(AVCodecContext *avctx, int16_t *samples,
//						int *frame_size_ptr,
//						AVPacket *avpkt);
typedef int (_cdecl *_FFAVCODEC_DECODE_AUDIO)(AVCodecContext *avctx, int16_t *samples,
											 					int *frame_size_ptr,
																AVPacket *avpkt);

//int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
//							int *got_frame_ptr, const AVPacket *avpkt);
typedef int (_cdecl *_FFAVCODEC_DECODE_AUDIO4)(AVCodecContext *avctx, AVFrame *frame,
													int *got_frame_ptr, const AVPacket *avpkt);


class FFCodec  
{
public:
	FFCodec();
	virtual ~FFCodec();

	static FFCodec *getInstance()
	{
		if(!theInstance)
			theInstance = new FFCodec;
		return theInstance;
	};
	
	AVCodec *ff_avcodec_find_decoder(enum CodecID id);
	AVCodec *ff_avcodec_find_encoder(enum CodecID id);
	AVCodecContext *ff_avcodec_alloc_context();
	AVFrame *ff_avcodec_alloc_frame();
	int ff_avcodec_open(AVCodecContext *context,AVCodec *code);
	int ff_avcodec_decode_video(AVCodecContext *context,AVFrame *frame,int *got_picture_ptr,const AVPacket *avpkt);
	int ff_avcodec_encode_video(AVCodecContext *context,unsigned char *buf, int buf_size, const AVFrame *pict);
	int ff_avcodec_close(AVCodecContext *context);
	void ff_av_freep(void *ptr);
	int ff_avpicture_deinterlace(AVPicture *dst, const AVPicture *src, int pix_fmt, int width, int height);
	int ff_avpicture_alloc(AVPicture *picture, int pix_fmt, int width, int height);
	void ff_av_init_packet(AVPacket *pkt);
	void ff_avcodec_free_frame(AVFrame **frame);
	void ff_av_free_packet(AVPacket *pkt);
	void ff_avcodec_flush_buffers(AVCodecContext *avctx);

	void* ff_av_mallocz(size_t size);

	int ff_avcodec_decode_audio(AVCodecContext *avctx, int16_t *samples,
								int *frame_size_ptr,
								AVPacket *avpkt);

	int ff_avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
		int *got_frame_ptr, const AVPacket *avpkt);

private:

	static FFCodec *theInstance;
	
	HINSTANCE avcodecDLLInstance;
	HINSTANCE avutilDLLInstance;

	//_FFAVCODEC_INIT ffavcodec_init;
	_FFAVCODEC_REGISTER_ALL ffavcodec_register_all;
	_FFAVCODEC_FIND_DECODER ffavcodec_find_decoder;
	_FFAVCODEC_FIND_ENCODER ffavcodec_find_encoder;
	_FFAVCODEC_ALLOC_CONTEXT ffavcodec_alloc_context;
	_FFAVCODEC_ALLOC_FRAME ffavcodec_alloc_frame;
	_FFAVCODEC_OPEN ffavcodec_open;
	_FFAVCODEC_DECODE_VIDEO ffavcodec_decode_video;
	_FFAVCODEC_ENCODE_VIDEO ffavcodec_encode_video;
	_FFAVCODEC_CLOSE ffavcodec_close;
	_FFAV_FREEP ffav_freep;
	_FFAVPICTURE_DEINTERLACE ffavpicture_deinterlace;
	_FFAVPICTURE_ALLOC ffavpicture_alloc;
	_FFAV_INIT_PACKET ffav_init_packet;
	_FFAVCODEC_FREE_FRAME ffavcodec_free_frame;
	_FFAVCODEC_FLUSH_BUFFERS ffavcodec_flush_buffers;
	_FFAV_FREE_PACKET ffav_free_packet;
	_FFAV_AV_MALLOCZ ffav_mallocz;
	_FFAVCODEC_DECODE_AUDIO ffavcodec_decode_audio;
	_FFAVCODEC_DECODE_AUDIO4 ffavcodec_decode_audio4;


	//void ff_avcodec_init();
	void ff_avcodec_register_all();
};

#endif // !defined(AFX_FFCODEC_H__8981C6ED_A189_4107_9B61_075B506B3A11__INCLUDED_)
