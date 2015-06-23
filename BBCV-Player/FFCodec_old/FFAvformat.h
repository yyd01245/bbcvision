#pragma once


#ifndef INT64_C
#define INT64_C(c) c##LL
#endif
#ifndef UINT64_C
#define UINT64_C(c) c##LL
#endif

#include "libavformat\avformat.h"
//#include "libavcodec\avcodec.h"
#include "windows.h"


//void av_register_all(void);
typedef int (_cdecl *_FFAV_REGISTER_ALL)();
//
////void av_open_input_file(void);
//typedef int (_cdecl *_FFAV_OPEN_INPUT_FILE)(AVFormatContext **, const char *, AVInputFormat *, int, AVFormatParameters *);
// int avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
typedef int (_cdecl *_FFAVFORMAT_OPEN_INPUT)(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);

////void av_open_input_stream(void);
//typedef int (_cdecl *_FFAV_OPEN_INPUT_STREAM)(AVFormatContext **, ByteIOContext *, const char *, AVInputFormat *, AVFormatParameters *);

//void av_find_stream_info(void);  avformat_find_stream_info
typedef int (_cdecl *_FFAV_FIND_STREAM_INFO)(AVFormatContext *ic);

//void av_init_packet(void);
//typedef int (_cdecl *_FFAV_INIT_PACKET)(AVPacket *pkt);

//void av_read_frame(void);
typedef int (_cdecl *_FFAV_READ_FRAME)(AVFormatContext *s, AVPacket *pkt);

//void av_close_input_file(AVFormatContext *s);
typedef int (_cdecl *_FFAV_CLOSE_INPUT_FILE)(AVFormatContext *s);
// void av_free_packet(AVPacket *pkt);
//typedef void (_cdecl *_FFAV_FREE_PACKET)(AVPacket *pkt);

//int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,int flags);
typedef int (_cdecl *_FFAV_SEEK_FRAME)(AVFormatContext *s, int stream_index, int64_t timestamp,int flags);

/*
AVIOContext *avio_alloc_context(
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
		int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
		int64_t (*seek)(void *opaque, int64_t offset, int whence));

*/

typedef (_cdecl *_FFAV_AVIO_ALLOC_CONTEXT)(		
												unsigned char *buffer,
											   int buffer_size,
											   int write_flag,
											   void *opaque,
											   int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
											   int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
											   int64_t (*seek)(void *opaque, int64_t offset, int whence));

//AVFormatContext *avformat_alloc_context(void);

typedef (_cdecl *_FFAV_AVFORMAT_ALLOC_CONTEXT)(void);

/*
int av_probe_input_buffer(AVIOContext *pb, AVInputFormat **fmt,
							const char *filename, void *logctx,
							unsigned int offset, unsigned int max_probe_size);
*/
typedef int (_cdecl *_FFAV_PROBE_INPUT_BUFFER)(AVIOContext *pb, AVInputFormat **fmt,
											   const char *filename, void *logctx,
											   unsigned int offset, unsigned int max_probe_size);
/*
void av_dump_format(AVFormatContext *ic,
					int index,
					const char *url,
					int is_output)
*/

typedef (_cdecl *_FFAV_DUMP_FORMAT)(AVFormatContext *ic,
									int index,
									const char *url,
									int is_output);



class FFAvformat
{
public:
	FFAvformat(void);
	~FFAvformat(void);

	static FFAvformat *getInstance()
	{
		if(!theInstance)
			theInstance = new FFAvformat;
		return theInstance;
	};

	void ff_av_register_all();
	int ff_avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
//	int ff_av_open_input_file(AVFormatContext **ic_ptr, const char *filename, AVInputFormat *fmt, int buf_size, AVFormatParameters *ap);
//	int ff_av_open_input_stream(AVFormatContext **ic_ptr, ByteIOContext *pb, const char *filename, AVInputFormat *fmt, AVFormatParameters *ap);
	int ff_av_find_stream_info(AVFormatContext *ic);
	//void ff_av_init_packet(AVPacket *pkt);
	int ff_av_read_frame(AVFormatContext *s, AVPacket *pkt);
	void ff_av_close_input_file(AVFormatContext *s);
	//void ff_av_free_packet(AVPacket *pkt);
	int ff_av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,int flags);

	AVIOContext * ff_avio_alloc_context(
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
		int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
		int64_t (*seek)(void *opaque, int64_t offset, int whence));

	AVFormatContext *ff_avformat_alloc_context(void);

	int ff_av_probe_input_buffer(AVIOContext *pb, AVInputFormat **fmt,
		const char *filename, void *logctx,
		unsigned int offset, unsigned int max_probe_size);

	int ff_av_dump_format(AVFormatContext *ic,
							int index,
							const char *url,
							int is_output);

	HINSTANCE dllInstance;

private:
	static FFAvformat *theInstance;

	_FFAV_REGISTER_ALL ffav_register_all;
	_FFAVFORMAT_OPEN_INPUT ffavformat_open_input;
	//_FFAV_OPEN_INPUT_FILE ffav_open_input_file;
	//_FFAV_OPEN_INPUT_STREAM ffav_open_input_stream;
	_FFAV_FIND_STREAM_INFO ffav_find_stream_info;
//	_FFAV_INIT_PACKET ffav_init_packet;
	_FFAV_READ_FRAME ffav_read_frame;
	_FFAV_CLOSE_INPUT_FILE ffav_close_input_file;
	//_FFAV_FREE_PACKET ffav_free_packet;
	_FFAV_SEEK_FRAME ffav_seek_frame;

	_FFAV_AVIO_ALLOC_CONTEXT ffavio_alloc_context;

	_FFAV_AVFORMAT_ALLOC_CONTEXT ffavformat_alloc_context;

	_FFAV_PROBE_INPUT_BUFFER ffav_probe_input_buffer;

	_FFAV_DUMP_FORMAT ffav_dump_format;

};
