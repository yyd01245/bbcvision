#include "Rtsp.h"

MyRTSPClient::MyRTSPClient()
{



}

MyRTSPClient::~MyRTSPClient()
{


}

int MyRTSPClient::RtspInit(char* strRtspUrl)
{
	//≥ı ºªØsocket
	SOCKET sock;
	int addr_len;
	int len;
	char UDP_buf[4096];
	FILE *fp;


	//fp = fopen("out.ts", "wb+");

/*	if ( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))  == -1) {
		//perror("socket");
		//exit(errno);
		printf("create socket.error \n\r");
		return -1;
	} else
		printf("create socket.\n\r");
*/
	sockaddr_in clientVideoService; 
	sockaddr_in c_addr; 
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int iPortAdd = 0;

	sockaddr_in clientVideoService2; 
	memset(&clientVideoService, 0, sizeof(struct sockaddr_in));
	clientVideoService.sin_family = AF_INET;

	clientVideoService.sin_port = htons(m_iRtspServerPort);


	clientVideoService.sin_addr.s_addr = INADDR_ANY;



	int nRecvBuf = 0;
	int iLen = 4;

	return 0;
}

int  MyRTSPClient::SendOptions(char *strOption,int ilen)
{

	return 0;
}

static unsigned int _stdcall MyRTSPClient::RecvRtsp_threadFun(void *param)
{
	
	return 0;
}

int MyRTSPClient::SendRequest(char *buff,int ilen)
{

	return 0;
}