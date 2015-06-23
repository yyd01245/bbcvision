#include <atlbase.h>
#include <string>
#include ".\ffavformat.h"

FFAvformat* FFAvformat::theInstance = 0;
FFAvformat::FFAvformat(void)
{

	DWORD err = 0; 

	char path[200] = {0};
	char temp[200] = {0};
	char strdll[20] = "avformat-54.dll";

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


	//avcodecDLLInstance = NULL;
	////avcodecDLLInstance =LoadLibrary("avcodec-54.dll");
	//avcodecDLLInstance =LoadLibrary(path);

	dllInstance = NULL;
	dllInstance = LoadLibrary(path);

	if(dllInstance != NULL)
	{
		//ffavcodec_init = NULL;
		//ffavcodec_init = (_FFAVCODEC_INIT)GetProcAddress(dllInstance,"avcodec_init");

		ffav_register_all = NULL;
		ffav_register_all = (_FFAV_REGISTER_ALL)GetProcAddress(dllInstance,"av_register_all");

		ffavformat_open_input = NULL;
		ffavformat_open_input = (_FFAVFORMAT_OPEN_INPUT)GetProcAddress(dllInstance,"avformat_open_input");

		ffav_find_stream_info = NULL;
		ffav_find_stream_info = (_FFAV_FIND_STREAM_INFO)GetProcAddress(dllInstance,"av_find_stream_info"); //av_find_stream_info

		//ffav_init_packet = NULL;
		//ffav_init_packet = (_FFAV_INIT_PACKET)GetProcAddress(dllInstance,"av_init_packet");

		ffav_read_frame = NULL;
		ffav_read_frame = (_FFAV_READ_FRAME)GetProcAddress(dllInstance,"av_read_frame");

		//ffav_open_input_stream = NULL;
		//ffav_open_input_stream = (_FFAV_OPEN_INPUT_STREAM)GetProcAddress(dllInstance,"av_open_input_stream");

		ffav_close_input_file = NULL;
		ffav_close_input_file = (_FFAV_CLOSE_INPUT_FILE)GetProcAddress(dllInstance,"av_close_input_file");

		//ffav_free_packet = NULL;
		//ffav_free_packet = (_FFAVCODEC_FREE_FRAME)GetProcAddress(dllInstance,"av_free_packet");

		ffav_seek_frame = NULL;
		ffav_seek_frame = (_FFAV_SEEK_FRAME)GetProcAddress(dllInstance,"av_seek_frame");

		ffavio_alloc_context =NULL;
		ffavio_alloc_context = (_FFAV_AVIO_ALLOC_CONTEXT)GetProcAddress(dllInstance,"avio_alloc_context");

		ffavformat_alloc_context = NULL;
		ffavformat_alloc_context = (_FFAV_AVFORMAT_ALLOC_CONTEXT)GetProcAddress(dllInstance,"avformat_alloc_context");

		ffav_probe_input_buffer = NULL;
		ffav_probe_input_buffer = (_FFAV_PROBE_INPUT_BUFFER)GetProcAddress(dllInstance,"av_probe_input_buffer");

		ff_av_register_all();
	}
}

FFAvformat::~FFAvformat(void)
{
	if(dllInstance)
	{
		FreeLibrary(dllInstance);
		dllInstance = NULL;
	}
}

void FFAvformat::ff_av_register_all()
{
	if(ffav_register_all)
		ffav_register_all();
}
int FFAvformat::ff_avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options)
{
	if(ffavformat_open_input)
		return ffavformat_open_input(ps,filename,fmt,options);
	else
		return -1;
}
//
//int FFAvformat::ff_av_open_input_stream(AVFormatContext **ic_ptr, ByteIOContext *pb, const char *filename, AVInputFormat *fmt, AVFormatParameters *ap)
//{
//	if (ffav_open_input_stream)
//		ffav_open_input_stream(ic_ptr, pb, filename, fmt, ap);
//	else
//		return -1;
//}

int FFAvformat::ff_av_find_stream_info(AVFormatContext *ic)
{
	if (ffav_find_stream_info)
		return ffav_find_stream_info(ic);
	else
		return -1;
}

//void FFAvformat::ff_av_init_packet(AVPacket *pkt)
//{
//	if (ffav_init_packet)
//		ffav_init_packet(pkt);
//}

int FFAvformat::ff_av_read_frame(AVFormatContext *s, AVPacket *pkt)
{
	if (ffav_read_frame)
		return ffav_read_frame(s, pkt);
	else 
		return -1;
}

void FFAvformat::ff_av_close_input_file(AVFormatContext *s)
{
	if (ffav_close_input_file)
		ffav_close_input_file(s);
}

int FFAvformat::ff_av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,int flags)
{
	if(ffav_seek_frame)
		return ffav_seek_frame(s,stream_index,timestamp,flags);
	else
		return -1;
}

AVIOContext * FFAvformat::ff_avio_alloc_context(
						  unsigned char *buffer,
						  int buffer_size,
						  int write_flag,
						  void *opaque,
						  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
						  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
						  int64_t (*seek)(void *opaque, int64_t offset, int whence))
{
	if(ffavio_alloc_context)
		return (AVIOContext *)ffavio_alloc_context(buffer,buffer_size,write_flag,opaque,read_packet,write_packet,seek);
	else
		return NULL;
}

AVFormatContext *FFAvformat::ff_avformat_alloc_context(void)
{
	if(ffavformat_alloc_context)
		return (AVFormatContext *)ffavformat_alloc_context();
	else
		return NULL;
}

int FFAvformat::ff_av_probe_input_buffer(AVIOContext *pb, AVInputFormat **fmt,
							 const char *filename, void *logctx,
							 unsigned int offset, unsigned int max_probe_size)
{
	if(ffav_probe_input_buffer)
		return ffav_probe_input_buffer(pb,fmt,filename,logctx,offset,max_probe_size);
	else
		return -1;
}

int FFAvformat::ff_av_dump_format(AVFormatContext *ic,
					   int index,
					   const char *url,
					   int is_output)
{
	if(ffav_dump_format)
		return ffav_dump_format(ic,index,url,is_output);
	else
		return -1;

}
