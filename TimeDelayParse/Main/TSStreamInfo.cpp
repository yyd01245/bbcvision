#include "TSStreamInfo.h"


#define __ES_PARSER_

const int TS_PACKET_SIZE = 188;

TSstreamInfo::TSstreamInfo()
{
	m_iAudioCodeType = STREAMTYPE_UNKNOWN;
	m_iVideoCodeType = STREAMTYPE_UNKNOWN;
	m_Mediafp = fopen("videoinfo.log","w");;
	m_bPesBeginFlag = false;
	m_iPMTPID = 0;
	m_iPCRPID = 0;
	m_iVideoPID = 0;
	m_iAudioPID = 0;
	m_FramDatafp = NULL;
	m_itsStreamPacketNumber = 0;
	m_iframesize = 0;
	m_iIframeSize = 0;
	m_bIFramebeginFlag = false;
		//获取当前时间作为后缀
	char tsFilePath[1024] = {0};
 	time_t now;
    struct tm tmnow;
    time(&now);
    localtime_r(&now,&tmnow);	
	sprintf(tsFilePath,"frameinfo_%d_%d_%d.log",tmnow.tm_hour,tmnow.tm_min,tmnow.tm_sec);
	printf("%s \n",tsFilePath);
	m_FramDatafp = fopen(tsFilePath,"wb");
	m_llFrameNum = 0;
	m_iFramTotal =	0;
	m_llFrameTotal = 0;
	m_iGopSize = 0;

	pthread_mutex_init(&m_mutexlocker,NULL);
}

TSstreamInfo::~TSstreamInfo()
{
	pthread_mutex_destroy(&m_mutexlocker);
}


void  TSstreamInfo::Adjust_PMT_table(TS_PMT* packet ,unsigned char *buffer)
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
/*
		if(packet->stream_type == 0x1B)
		{
			// video h264
			m_iVideoPID = packet->elementary_PID;
		}
		else if(packet->stream_type == 0x03)
		{
			m_iAudioPID = packet->elementary_PID;
		}
*/
		switch(packet->stream_type)
		{
		  case STREAMTYPE_11172_AUDIO:
	      case STREAMTYPE_13818_AUDIO:
	      case STREAMTYPE_AC3_AUDIO:
	      case STREAMTYPE_AAC_AUDIO:
	      case STREAMTYPE_MPEG4_AUDIO:
  			{
				m_iAudioCodeType = packet->stream_type;
				m_iAudioPID = packet->elementary_PID;
				printf("---get audio pid %d codetype %d \n",m_iAudioPID,m_iAudioCodeType);
	            break;
	      	}
	      case STREAMTYPE_13818_VIDEO:	  	
	      case STREAMTYPE_11172_VIDEO:
	      case STREAMTYPE_H264_VIDEO:
	      case STREAMTYPE_AVS_VIDEO:
		  	{
				m_iVideoCodeType = packet->stream_type;
				m_iVideoPID = packet->elementary_PID;
				printf("---get video pid %d codetype %d \n",m_iVideoPID,m_iVideoCodeType);
	            break;
	      	}	
	      case STREAMTYPE_13818_PES_PRIVATE:
	             break;
	      case STREAMTYPE_13818_B:
	             break;
	      default:
	             break;
		
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


int  TSstreamInfo::Adjust_PAT_table(TS_PAT* packet ,unsigned char *buffer)
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


void TSstreamInfo::Adjust_TS_packet_header(TS_packet_Header* pHeader,unsigned char *buffer)
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


int64_t TSstreamInfo::Get_PCR_Value(unsigned char* buffer,int itemplen)
{
	int64_t ret = 0;

	return ret;
}
int TSstreamInfo::Set_PCR_Value(unsigned char* buffer,int index,int64_t pcr_value)
{
	printf("---set pcr value = %lld \n",pcr_value);

	unsigned char* bufftmp = buffer+index;
	int64_t pcr_low = pcr_value % 300, pcr_high = pcr_value / 300;

    *bufftmp++ = pcr_high >> 25;
    *bufftmp++ = pcr_high >> 17;
    *bufftmp++ = pcr_high >>  9;
    *bufftmp++ = pcr_high >>  1;
    *bufftmp++ = pcr_high <<  7 | pcr_low >> 8 | 0x7e;
    *bufftmp++ = pcr_low;
	
	return 0;
}
int TSstreamInfo::Find_PCR_Index(unsigned char* buff,int ilen,int *iIndex)
{
	//识别是否ts头
	int itsHead;
	int index = 0;
	unsigned char *packet;

	int iseekLen = 0; //ts head len
	int iRetPCRindex = -1;

	bool hasFindpcr = false;
	while(index <= ilen-TS_PACKET_SIZE && !hasFindpcr)
	{
		if(buff[index]==0x47 && 
			((index< ilen - TS_PACKET_SIZE) ? (buff[index+TS_PACKET_SIZE]==0x47):1))
		{			
			packet = &buff[index];
			TS_packet_Header tmpPacketHeader;
			Adjust_TS_packet_header(&tmpPacketHeader,packet);

			int pid = tmpPacketHeader.PID ;
			int iPoint_fielLen = tmpPacketHeader.payload_unit_start_indicator;

			if(pid == 0x1fff)
			{
				//NULL packet
				//跳过这个包
				//printf("---no palyload data \n");
				index += TS_PACKET_SIZE;
				continue;
			}
			if (tmpPacketHeader.payload_unit_start_indicator != 0x01) // 表示不 含有PSI或者PES头
			{
				//printf("---no start data \n");
				//跳过这个包
				index += TS_PACKET_SIZE;
				continue;
			}
			int len, cc,  afc, is_start, is_discontinuity,
				has_adaptation, has_payload;
			afc = tmpPacketHeader.adaption_field_control;
			uint8_t *p, *p_end,*pEsData;

			p = packet + 4;
			if (afc == 0) /* reserved value */
				return 0;
			has_adaptation = afc & 2;
			has_payload = afc & 1;
	//		is_discontinuity = has_adaptation
	//			&& packet[4] != 0 /* with length > 0 */
	//			&& (packet[5] & 0x80); /* and discontinuity indicated */

			cc = (packet[3] & 0xf);

			if (has_adaptation) 
			{
				/* skip adaptation field */
				//p += p[0] + 1;  p[0]为调整字段长度
				int iPCRflag = p[1]&0x10;
				if(iPCRflag ==0x10)
				{
					iRetPCRindex = index+4+2;
					//pcrflag 
					uint64_t pcr_high =(uint64_t)(p[2] << 25);
					pcr_high = pcr_high | (p[3] << 17);
					pcr_high = pcr_high | (p[4] << 9);
					pcr_high = pcr_high | (p[5] << 1);
					pcr_high = pcr_high | (p[6]&0x80)>>7;

					uint64_t pcr_low = (uint64_t)((p[6]&0x01)<<8);
					pcr_low == pcr_low | p[7];

					m_llPCR = pcr_high*300 + pcr_low;
					*iIndex = iRetPCRindex;
					printf("---pcr value = %lld \n",m_llPCR);
					printf("----pcr1%x %x %x %x \n",buff[iRetPCRindex],buff[iRetPCRindex+1],
							buff[iRetPCRindex+2],buff[iRetPCRindex+3]);

					//找到pcr
					return 0;

					//m_llPCR = m_llPCR /300; //换成90khz
				}

		//		p += p[0] + 1; // p[0]为调整字段长度
				/* if past the end of packet, ignore */
		//		p_end = packet + TS_PACKET_SIZE;
		//		if (p >= p_end)
		//			return 0;
			}
			
			if(!hasFindpcr)
				index += TS_PACKET_SIZE;
		}
		else
			++index;
	}
	return -1;
}


uint64_t TSstreamInfo::Parse_PTS(unsigned char *pBuf)
{
	unsigned long long llpts = (((unsigned long long)(pBuf[0] & 0x0E)) << 29)
		| (unsigned long long)(pBuf[1] << 22)
		| (((unsigned long long)(pBuf[2] & 0xFE)) << 14)
		| (unsigned long long)(pBuf[3] << 7)
		| (unsigned long long)(pBuf[4] >> 1);
	return llpts;
}


bool TSstreamInfo::GetVideoESInfo(unsigned char *pEsData,int itempLen)
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


bool TSstreamInfo::Find_Stream_IFrame(unsigned char *buff,int ilen,int* pIndex)
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
			hasFindIframe = false;
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
				//printf("---no palyload data \n");
				index += TS_PACKET_SIZE;
				continue;
			}
			if (tmpPacketHeader.payload_unit_start_indicator != 0x01) // 表示不 含有PSI或者PES头
			{
				//printf("---no palyload data \n");
				fflush(m_Mediafp);
				//跳过这个包
				index += TS_PACKET_SIZE;
				continue;
			}
	
			int len, cc,  afc, is_start, is_discontinuity,
				has_adaptation, has_payload;
			afc = tmpPacketHeader.adaption_field_control;
			uint8_t *p, *p_end,*pEsData;


			if(m_iPMTPID == 0 && pid ==0 )
			{
				// PAT 找到PMT信息
				//8+ 1+1+2+12+16+2+5+1+8+8
				//section_length 
				//如果有调整字段 需要重新锁定

				int ihas_adaptation = afc & 2;
				int ihas_payload = afc & 1;
							
				TS_PAT tmpTSPat;
				memset(&tmpTSPat,0,sizeof(tmpTSPat));
				int iret = Adjust_PAT_table(&tmpTSPat,packet+4+iPoint_fielLen);  //跳过一个字节指针域
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
				Adjust_PMT_table(&tmpPMT,packet+4+iPoint_fielLen);
				
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
				if(hasFindIframe)
					*pIndex = index;
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

bool TSstreamInfo::Find_IFrame(unsigned char *p,int iseekLen)
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


bool TSstreamInfo::Adjust_PES_Pakcet(unsigned char *p,int iseekLen)
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
		printf("-----------pes len =%d\n",ilen);
		
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
			//m_iFramTotal++;
			fprintf(m_Mediafp,"PCR=%d \n",m_llPCR);
			fprintf(m_Mediafp,"PTS=%d \n",m_llPts);
			fprintf(m_Mediafp,"PTS-PCR=%d \n",m_llPts-m_llPCR);
			if(m_llPts - m_llLastPts > 0)
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

		// 方法一: 被pes长度所限制，不准确
		//确认是收到视频PES 的头数据即为一帧开始。
		//确定PES长度，进行计算当前接收的数据长度
		//达到此长度，则认为一帧数据已接收完全。

		//方法二: 
		//确认收到的是一帧开始，并做一个标识
		//接收数据如果是其他数据的开始则认为该PES数据接收结束
		pthread_mutex_lock(&m_mutexlocker);
		m_bPesBeginFlag = true;
		
		printf("-------------num = %ld \n",m_llFrameNum);
		m_llFrameNum++;
		pthread_mutex_unlock(&m_mutexlocker);
	
		//return GetVideoESInfo(pEsData,itempLen);
#ifdef __ES_PARSER_		
		return ParseH264ES(pEsData,itempLen);
#else
		return true;
#endif
	}

	return false;
}


//FILE* fptest = NULL;
bool TSstreamInfo::ParseH264ES(unsigned char* buffer,int itemplen)
{
	int buf_index = 0;
	int next_avc = itemplen;
	unsigned char * pData = NULL;
//	if(fptest == NULL)
//		fptest = fopen("test.264","wb+");
	bool bSPSFlag = false;
	bool bPPSFlag = false;

	bool bSeqHead = false;
	bool bSeqExten = false;
	bool bGropFlag = false;
	
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

		if(m_iVideoCodeType == STREAMTYPE_13818_VIDEO )
		{
			printf("---mpeg2 stream \n");
			if(*(pData) == 0xb8)
			{
				// b5是P帧
				//mpeg2
							// I frame
				m_iGopSize++;
				//将gopsize 清空
				//if(m_iGopSize != 0)
				//{
					//记录gopsize
					fprintf(m_Mediafp," MPEG2 Gop index=%d gopsize=%d\n",m_iGopSize,m_llFrameNum-m_iFramTotal);
					fflush(m_Mediafp);
			
				//	fwrite(pData,1,itemplen-5-buf_index,fptest);
				//}
				//m_iGopSize = 0;
				m_iFramTotal = m_llFrameNum;
				break;
			}
			else if((*(pData) == 0x00))
			{
				m_iVideoCodeType = STREAMTYPE_13818_VIDEO;
				//图像起始码 
				// 跳过temporal_reference 10位 
				//Picture_coding_type（3位）,如果Picture_coding_type为
				//001（二进制）就是I帧，为010是P帧,011则就 是B帧,
				unsigned char * pstartcode = pData;
				pstartcode += 1;
				//跳过前2位，取到中间三位
				int ipic_code_type = ((*pstartcode) << 2 >>2) & 0x38;
				if(ipic_code_type == 2 || ipic_code_type == 3)
				{
					//m_iGopSize++;
					fprintf(m_Mediafp," MPEG2 PB Frame=%d \n",ipic_code_type);
					fflush(m_Mediafp);
				}
			}

		}
		else if(m_iVideoCodeType == STREAMTYPE_H264_VIDEO )
		{
			//printf("---h264 stream \n");
			if(iflag == 5)
			{
				// I frame
				
				//将gopsize 清空
				//if(m_iGopSize != 0)
				{
					//记录gopsize
					printf("---last framnum =%d \n",m_iFramTotal);
					fprintf(m_Mediafp,"  Gop index=%d gopsize=%d\n",m_iGopSize,m_llFrameNum-m_iFramTotal);
					fflush(m_Mediafp);

				//	fwrite(pData,1,itemplen-5-buf_index,fptest);
				}
				//m_iGopSize = 0;
				m_iGopSize++;
				m_iFramTotal = m_llFrameNum;

				m_bIFramebeginFlag = true;
				break;
			}
			else if(iflag==1)
			{
				//m_iGopSize++;
				break;
			}
		}
		else
		{
			//PMT表没有先来，或者解析不正确
			if(*(pData) == 0xb3)
				bSeqHead = true;
			if(*(pData) == 0xb5)
				bSeqExten = true;
			if(bSeqExten && bSeqHead)
				m_iVideoCodeType = STREAMTYPE_13818_VIDEO;
			if(bSeqExten && bSeqHead && (*(pData) == 0xb8))
			{
				// b5是P帧
				//mpeg2
							// I frame
				m_iVideoCodeType = STREAMTYPE_13818_VIDEO;
				m_iGopSize++;
				//将gopsize 清空
				//if(m_iGopSize != 0)
				//{
					//记录gopsize
					fprintf(m_Mediafp," MPEG2 Gop index=%d gopsize=%d\n",m_iGopSize,m_llFrameNum-m_iFramTotal);
					fflush(m_Mediafp);
			
				//	fwrite(pData,1,itemplen-5-buf_index,fptest);
				//}
				//m_iGopSize = 0;
				m_iFramTotal = m_llFrameNum;
				break;
			}
			else if(bSeqExten && bSeqHead && (*(pData) == 0x00))
			{
				m_iVideoCodeType = STREAMTYPE_13818_VIDEO;
				//图像起始码 
				// 跳过temporal_reference 10位 
				//Picture_coding_type（3位）,如果Picture_coding_type为
				//001（二进制）就是I帧，为010是P帧,011则就 是B帧,
				unsigned char * pstartcode = pData;
				pstartcode += 1;
				//跳过前2位，取到中间三位
				int ipic_code_type = ((*pstartcode) << 2 >>2) & 0x38;
				if(ipic_code_type == 2 || ipic_code_type == 3)
				{
					//m_iGopSize++;
					fprintf(m_Mediafp," MPEG2 PB Frame=%d \n",ipic_code_type);
					fflush(m_Mediafp);
				}
			}
			if(m_iVideoCodeType != STREAMTYPE_13818_VIDEO)
			{
				printf("---h264 stream \n");
				// h264
				if(iflag == 7)
					bSPSFlag = true;
				if(iflag == 8)
					bPPSFlag = true;
				if(bSeqHead && bPPSFlag)
				{
					if(iflag == 5)
					{
						// I frame
						m_iGopSize++;
						{
							//记录gopsize
							fprintf(m_Mediafp,"  Gop index=%d gopsize=%d\n",m_iGopSize,m_llFrameNum-m_iFramTotal);
							fflush(m_Mediafp);
						
						//	fwrite(pData,1,itemplen-5-buf_index,fptest);
						}
						//m_iGopSize = 0;
						m_iFramTotal = m_llFrameNum;
						m_bIFramebeginFlag = true; //I 帧标识
						break;
					}
					else if(iflag==1)
					{
						//m_iGopSize++;
						break;
					}
				}
			}
		}
			

	}


	// 
	return true;
}



int TSstreamInfo::ParseStreamInfo(uint8_t *buff,int ilen)
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
			if (tmpPacketHeader.payload_unit_start_indicator != 0x01) // 表示不 含有PSI或者PES头
			{
				printf("---no palyload dat \n");
				fflush(stdout);
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
			 case 0x0:									  // reserved for future use by ISO/IEC
				 return false;
			 case 0x1:									  // 无调整字段，仅含有效负载  
				 //iBeginlen += packet[iBeginlen] + 1;	// + pointer_field
				 iBeginlen +=  0;  // + pointer_field
				 break;
			 case 0x2:									   // 仅含调整字段，无有效负载
			 //  iBeginlen += packet[iBeginlen] + 1;  // + pointer_field
				 iBeginlen += 0;  // + pointer_field
				 break;
			 case 0x3:									  // 调整字段后含有效负载
				 if (adaptation_field_length > 0) 
				 {
					 iBeginlen += 0;				   // adaptation_field_length占8位
					 iBeginlen += adaptation_field_length; // + adaptation_field_length
				 }
				 else
				 {
					 iBeginlen += 0;
				 }
				 //iBeginlen += packet[iBeginlen] + 1;			 // + pointer_field
				 break;
			 default:	 
				 break;
			 
			 }


			if(m_iPMTPID == 0 && pid ==0 )
			{
				// PAT 找到PMT信息
				//8+ 1+1+2+12+16+2+5+1+8+8
				//section_length 
				//如果有调整字段 需要重新锁定

				int ihas_adaptation = afc & 2;
				int ihas_payload = afc & 1;
							
				TS_PAT tmpTSPat;
				memset(&tmpTSPat,0,sizeof(tmpTSPat));
				int iret = Adjust_PAT_table(&tmpTSPat,packet+4+iPoint_fielLen);  //跳过一个字节指针域
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
				Adjust_PMT_table(&tmpPMT,packet+4+iPoint_fielLen);
				
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


//解析一个TS包188 bytes，找到是否有视频或音频的结束标识。
int TSstreamInfo::ParseStreamFrame(uint8_t *buff,int ilen)
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

	bool hasFindFrameFlag = false;
	bool hasFindIframe = false;
	bool hasFindSPS = false;
	bool hasFindPPS = false;
	while(index < ilen && !hasFindFrameFlag)
	{
		if(buff[index]==0x47)
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
			if((m_iVideoPID == pid) && m_bPesBeginFlag)
			{
				++m_itsStreamPacketNumber;
				//printf("---packet video %d\n",m_itsStreamPacketNumber);
			}
			if (tmpPacketHeader.payload_unit_start_indicator != 0x01) // 表示不 含有PSI或者PES头
			{
				//printf("---no palyload dat %s \n",__FUNCTION__);
				//fflush(stdout);

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
			 case 0x0:									  // reserved for future use by ISO/IEC
				 return false;
			 case 0x1:									  // 无调整字段，仅含有效负载  
				 //iBeginlen += packet[iBeginlen] + 1;	// + pointer_field
				 iBeginlen +=  0;  // + pointer_field
				 break;
			 case 0x2:									   // 仅含调整字段，无有效负载
			 //  iBeginlen += packet[iBeginlen] + 1;  // + pointer_field
				 iBeginlen += 0;  // + pointer_field
				 break;
			 case 0x3:									  // 调整字段后含有效负载
				 if (adaptation_field_length > 0) 
				 {
					 iBeginlen += 0;				   // adaptation_field_length占8位
					 iBeginlen += adaptation_field_length; // + adaptation_field_length
				 }
				 else
				 {
					 iBeginlen += 0;
				 }
				 //iBeginlen += packet[iBeginlen] + 1;			 // + pointer_field
				 break;
			 default:	 
				 break;
			 
			 }

			if(m_iPMTPID == 0 && pid ==0 )
			{
				// PAT 找到PMT信息
				//8+ 1+1+2+12+16+2+5+1+8+8
				//section_length 
				//如果有调整字段 需要重新锁定

				int ihas_adaptation = afc & 2;
				int ihas_payload = afc & 1;
							
				TS_PAT tmpTSPat;
				memset(&tmpTSPat,0,sizeof(tmpTSPat));
				int iret = Adjust_PAT_table(&tmpTSPat,packet+4+iPoint_fielLen);  //跳过一个字节指针域
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
				Adjust_PMT_table(&tmpPMT,packet+4+iPoint_fielLen);
				
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


			if(packet[1] & 0x40 && (m_iVideoPID == pid))  //is_start = packet[1] & 0x40;
			{	
								
				//查找 pes 
				iseekLen = p - packet;
				//如果pes标识为true,说明一帧结束
				if(m_bPesBeginFlag == true)
				{
					m_iframesize = (184*m_itsStreamPacketNumber-iseekLen); //由于有填充FF空数据故会不够准确。
					m_itsStreamPacketNumber = 0;
					RecordVideoInfo();
					if(m_bIFramebeginFlag == true)
					{
						m_iIframeSize = m_iframesize;
						printf("I frame size =%d \n",m_iIframeSize);
						m_bIFramebeginFlag = false;
					}
					else
					{
						printf("P frame size=%d \n",m_iframesize);
					}
				}

				hasFindIframe = Adjust_PES_Pakcet(p,iseekLen);
				
			}

			index += TS_PACKET_SIZE;
//			if(!hasFindIframe)
//				index += TS_PACKET_SIZE;
		}
		else
		{
			index++;
		}

	}	
	return 0;
}

void TSstreamInfo::get_time(char*buff,int siz)
{
    char lbuf[32];
    struct tm tmnow;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    localtime_r(&ts.tv_sec,&tmnow);
    strftime(lbuf,31,"%Y/%m/%d-%H:%M:%S",&tmnow);
    snprintf(buff,siz,"%s.%03ld",lbuf,(ts.tv_nsec+500000)/1000000);

}



int TSstreamInfo::RecordVideoInfo()
{
	//记录当前绝对时间
	
	//将标识恢复为false
	pthread_mutex_lock(&m_mutexlocker);
	m_bPesBeginFlag = false;

	char txtinfo[48] ={0};
	get_time(txtinfo,48);

	//printf("%s \n",txtinfo);
	
	char strFramInfo[128] = {0};
#if 1
	sprintf(strFramInfo,"%s \t index:%d \n",txtinfo,m_llFrameTotal);
	printf("%s",strFramInfo);
	fwrite(strFramInfo,1,strlen(strFramInfo),m_FramDatafp);
	fflush(m_FramDatafp);
	m_llFrameTotal = m_llFrameNum;
	//m_llFrameTotal++;
#endif

/*	
	sprintf(strFramInfo,"%s \t index:%d \n",txtinfo,m_llFrameNum);
	printf("%s",strFramInfo);
	fwrite(strFramInfo,1,strlen(strFramInfo),m_FramDatafp);
	fflush(m_FramDatafp);
*/
	pthread_mutex_unlock(&m_mutexlocker);
	return 0;
}


