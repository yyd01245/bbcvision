// RTSP Client Header File

#ifndef RTSPCH_
#define RTSPCH_

#define RTSP_SEVER_PORT 556		// Well-known RTSP server port
#define MAX_STR			1024	// Max string lenget
#define MAX_BUF			1024	// Size of receive buffer
#define MAX_LINE		1024	// No single line in the response should be more than
#define MAX_RESP_SIZE	65536
#define MAX_MEDIA		6		// This many media may be present (all-media, audio, video, text, ...)

// State definitions

#define IDLE		0
#define PLAYING		1
#define PAUSED		2

#define DESCRIBE	1
#define SETUP		2
#define PLAY		3
#define PAUSE		4
#define TEARDOWN	5


/*********************
		      WIN32 compatibility */

#ifdef UNIX
#define MB_OK 0
#define SOCKET_ERROR -1
#define SOCKET int
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define ZeroMemory bzero
#define TRUE 1
#define FALSE 0
#endif



struct SESSION_MEDIA {
	char setupInfo[MAX_STR]; // :video, :audio etc.
	int uniPort;
	int multiPort;
	int multiTTL;
	char srcAddr[MAX_STR];
	char type[MAX_STR]; // audio, video, application ...
};

#ifdef UNIX
struct PROCESS_INFORMATION {
  int pid;
};

#include <sys/socket.h>
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <process.h>


#define MESS_PLAY_STATUS_BYE	WM_USER + 100


typedef struct __RTCPHead
{
	//part1
	unsigned Version					:2;
	unsigned PayloadFlag				:1;
	unsigned RecvCounter				:5;
	unsigned PayloadType				:8;
	unsigned Length						:16;
	unsigned Sequence					:16;
	unsigned SSRCInfo					:32;

	//part2 为发送者信息
	unsigned NTPTimestamphigh			:32;
	unsigned NTPTimestamplow			:32;

	unsigned RTPTimestamp				:32;
	unsigned SenderCounter				:32;
	unsigned SenderOctetCounter			:32;

	//part3 为接收报告块 大小不固定

	//unsigned CSRCList				
}RTCPHead;


class CRTSPclient {
public:
	CRTSPclient ();

	int Connect (const char* severURL);
	int Pause ();
	int Resume ();
	int Disconnect ();
	const char* GetTitle ();

	int GetRtspPort();
	int GetRtpServerPort();

	int TestRTPLinkRight();

	int ParseRTCPInfo(char *buff,int ilen);
	static unsigned int _stdcall Rtcp_Data_recv(void* param);

	int SetCheckThreadID(unsigned udp_recv_thread);
private:

	unsigned m_Check_threadid;
	

	unsigned m_RtcpThreadID;
	bool m_bstop;
	int m_iRtspDataPort; //rtp的端口接收媒体流
	int m_iRtcpPort;	//rtcp的端口

	char m_strRTPServerIP[256];
	int m_iRTPServerPort;

	int state; // defined above
	char host[MAX_STR]; // DNS host name
	int port;
	char file[MAX_STR]; // SDP file name, session description
	SOCKET serv_socket;
	int cSeq;	// Increased by 1, always
	char session[MAX_STR]; // Unique RTSP session identifier
	char sessName[MAX_STR]; // Session name extracted from SDP
	char accept[MAX_STR];		// We can parse only SDP
	SESSION_MEDIA media[MAX_MEDIA]; // We support up to MAX_MEDIA descriptions
			// media[0] is reserved for "all media"
	int numMedia;			// ... and we actually have this many
	PROCESS_INFORMATION mph[MAX_MEDIA];		// media process handles for vic, rat etc.
	int lastPort;


	int ServConnect (const char* serverURL);
	int GetSomePort ();
	int ParseInput (const char* serverURL);
	int SendRequest (int type, int media); // e.g. (SETUP, 0)
	int RecvResponse (int type, int mediaCtrl);
	int readLine (char* from_str, int first, int max, char* to_str);
};

#endif