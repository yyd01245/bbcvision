#include "TSDecoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>


const int BUFFSIZE = 1280*720*3/2;

FILE *fp =NULL;
int main()
{
	int iwidth = 0;
	int iHeight = 0;
	//const char* filename = "/home/ky/rsm-yyd/DecoderTs/1.ts";
	TSDecoder_t* pInstance = NULL;
	pInstance = init_TS_Decoder("udp://@:55555");

	//pInstance = init_TS_Decoder("/home/ky/rsm-yyd/DecoderTs/dsa.ts",iwidth,iHeight);
	//
	//usleep(40*1000);


	//udp @ 0x7facd4000e60] bind failed: Address already in use

	int output_video_size = BUFFSIZE;
	unsigned char *output_video_yuv420 = new unsigned char[BUFFSIZE];

	int input_audio_size = 1024*100;
	unsigned char *output_audio_data = new unsigned char[1024*100];
//	fp = fopen("/home/ky/rsm-yyd/DecoderTs/overlay/yuv","wb+");
//	{
//		if(NULL == fp)
//			return -1;
//	}

//	FILE *fpaudio = fopen("/home/ky/rsm-yyd/DecoderTs/overlay/audio.pcm","wb+");
//	if(NULL == fpaudio)
//		return -1;
	
	unsigned long ulpts = 0;
	unsigned long audio_pts = 0;
	int iloop = 25*20000;
	while(1)
		{
			usleep(39*1000);
/*			output_video_size = BUFFSIZE;
			int ret = get_Video_data(pInstance,output_video_yuv420,&output_video_size,&iwidth,&iHeight,&ulpts);
			if(--iloop < 0){
				Set_tsDecoder_stat(pInstance,true);
				
			}
			if(ret ==0)
			{
				//do
				//{
					ret = get_Audio_data(pInstance,output_audio_data,&input_audio_size,&audio_pts);
					//fwrite(output_audio_data,1,input_audio_size,fpaudio);
					printf("----audio outputsize=%d,pts =%d,ret=%d\n",input_audio_size,audio_pts,ret);
				//}while(ret ==0);
				
				fprintf(stderr,"video outputsize=%d ,w=%d,h=%d audiopts=%d ,videopts=%d\n",output_video_size,iwidth,iHeight,audio_pts,ulpts);
			//fwrite(output_video_yuv420,1,output_video_size,fp);
				iwidth=0;
				iHeight=0;
			}
*/
		}
	return 0;
	}
