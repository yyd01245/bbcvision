//
// RTSP Client Implementation
//
// Tarik Cicic, November 1999 - March 2000
//

//#ifdef WINS

#include "stdafx.h"
//#include "Rtspc.h"
#include "BBCV-Player.h"

//#endif


#ifdef UNIX 
 
#ifdef SOLAR
#include <strings.h>
#include <sys/types.h>
#endif

#ifdef LINUX
#include <string.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>    
#include <netdb.h>    
#include <unistd.h> 

#endif

#include <stdlib.h>
#include "RTSPclient.h"


#ifdef WINS
extern CBBCVPlayerApp theApp;
//extern CRtspcApp theApp; // We will have use for this one.
#endif


#ifdef UNIX
void MessageBox (void *, const char* msg, const char* title, int) {
  fprintf (stderr, "%s: %s\n", title, msg);
}
#endif

#pragma comment(lib,"ws2_32.lib")

CRTSPclient::CRTSPclient () {
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD(1, 1);
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		MessageBox(NULL, "Could not initialize sockets,\nRtspc cannot start!", NULL, MB_OK);
		exit (1);	
	}
 
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		    HIBYTE( wsaData.wVersion ) != 1 ) {
		MessageBox(NULL, "Could not find WinSock1.1,\nRtspc cannot start!", NULL, MB_OK);                                
		WSACleanup( );
		exit (1);
	}
#endif
	// Other initialization
	cSeq = 0;
	lastPort = 5000;
	state = IDLE;
	memset(session,0,sizeof(session));
	memset(sessName,0,sizeof(sessName));

	memset(host,0,sizeof(host));
	memset(file,0,sizeof(file));
	memset(accept,0,sizeof(accept));
	memset(media,0,sizeof(media));
	memset(mph,0,sizeof(mph));
	m_Check_threadid = 0;
	m_iRTPServerPort = 0;
}

int CRTSPclient::ParseRTCPInfo(char* buff,int ilen)
{
	RTCPHead tmpRtcphead;
	memset(&tmpRtcphead,0,sizeof(tmpRtcphead));
	char firstChar = *buff;
	tmpRtcphead.Version = firstChar >> 6;
	tmpRtcphead.PayloadType = *(buff+1);
	if(tmpRtcphead.PayloadType == 200)
	{
		//SR
	}
	tmpRtcphead.Length = (unsigned int)(*(buff+2) << 8);
	tmpRtcphead.Length = tmpRtcphead.Length | (*(buff+3));

	int iheadlen = 4*(tmpRtcphead.Length+1);

	//SR之后就是对象信息
	char *pNewBuff = buff + iheadlen;
	
	RTCPHead tmpRtcphead2;
	memset(&tmpRtcphead2,0,sizeof(tmpRtcphead2));

	tmpRtcphead2.Version = (*pNewBuff) >> 6;
	tmpRtcphead2.PayloadType = *(pNewBuff+1);
	if(tmpRtcphead2.PayloadType == 203)
	{
		//goodbye
		printf("---goodbye\n");
		return true;
	}

	

	return false;
}

//rtcp thread
unsigned int _stdcall CRTSPclient::Rtcp_Data_recv(void* param)
{
	CRTSPclient* this0 = (CRTSPclient*)param;

//	this0->udp_recv_thread = GetCurrentThreadId();

	SOCKET sock;
	int addr_len;
	int len;
	char UDP_buf[4096];
	FILE *fp;

	sockaddr_in clientVideoService; 
	sockaddr_in c_addr; 
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int iPortAdd = 0;

	sockaddr_in clientVideoService2; 
	memset(&clientVideoService, 0, sizeof(struct sockaddr_in));
	clientVideoService.sin_family = AF_INET;
	clientVideoService.sin_port = htons(this0->m_iRtcpPort);
	clientVideoService.sin_addr.s_addr = INADDR_ANY;

	int nRecvBuf = 0;
	int iLen = 4;
	setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (char*)&nRecvBuf, sizeof(int) );

	if ( (bind(sock, (struct sockaddr*)&clientVideoService, sizeof(clientVideoService))) == -1 ) {
	//	perror("bind");
	//	exit(errno);
	}else
		printf("bind address to socket.\n\r");

	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nRecvBuf, &iLen);
	nRecvBuf = 1024*1024;//设置为32K
	setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	int nSize = 0;
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, &iLen);

	
	addr_len = sizeof(struct sockaddr);
	bool bneedwait = false;

	int iFilterLen = 0;

	while(!this0->m_bstop) {
		len = recvfrom(sock, UDP_buf, sizeof(UDP_buf)-1, 0, (struct sockaddr*)&c_addr, &addr_len);
		
		int ret = this0->ParseRTCPInfo(UDP_buf,len);
		if(ret&& this0->m_Check_threadid != 0)
		{
			PostThreadMessage(this0->m_Check_threadid,MESS_PLAY_STATUS_BYE,NULL,NULL);
			break;
		}
	}	

	return 0;
}

int CRTSPclient::SetCheckThreadID(unsigned udp_recv_thread)
{
	m_Check_threadid =udp_recv_thread;
	return 0;
}

/*********************'
	ServConnect: parse server URL, complete host, port and file information and
	create the serv_socket
**********/

int CRTSPclient::ServConnect (const char * serverURL) {
	int res = 0;
	unsigned int addr;
	int socket_type = SOCK_STREAM;
	struct sockaddr_in server;
	struct hostent *hp;

	if (state != IDLE) {
		Disconnect ();
		state = IDLE;
	}
	strcpy (session, ""); 
	strcpy (sessName, ""); 
	strcpy (accept, "");	
	numMedia = 0;	

	if ((res = ParseInput (serverURL)) != 0) 
		MessageBox(NULL, "Invalid server URL!", "Network Error", MB_OK);
	else {
		// Good URL, stored in the host, port and file globals
		if (isalpha(host[0])) {   /* server address is a name */
			hp = gethostbyname(host);
		}
		else  { /* Convert nnn.nnn address to a usable one */
			addr = inet_addr(host);
	//		hp = gethostbyaddr((char *)&addr,4,AF_INET);
		}
//		if (hp == NULL ) {
//			MessageBox(NULL, "Host not found!", "Network Error", MB_OK);
//			return 10;
//		}

		memset(&server,0,sizeof(server));
		//memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = inet_addr(host);

		serv_socket = socket(AF_INET,socket_type,0); /* Open the socket */
		if (serv_socket <0 ) {
			MessageBox(NULL, "Could not open the socket!", "Network Error", MB_OK);
			return 11;
		}
 
		if (connect(serv_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
			MessageBox(NULL, "Could not connect to the server!", "Network Error", MB_OK);
			return 12;
		}
	}

	return res;
}

int CRTSPclient::Connect (const char * serverURL) {
	int i, res = 0;
	char command[256];
	char str[64];
	m_bstop = false;

	if ((res = ServConnect (serverURL)) == 0) {		
		if (SendRequest (DESCRIBE, 0) != 0) {
			MessageBox(NULL, "Failed sending DESCRIBE request!", "Protocol Error", MB_OK);
			return 20;
		}
		else {
			if (RecvResponse (DESCRIBE, 0) != 0) 
				return 30;
		}
		if (numMedia == 0) {
			if (SendRequest(SETUP, 0) != 0) {
				MessageBox(NULL, "Failed sending SETUP request!", "Protocol Error", MB_OK);
				return 21;
			}
			else{
				if (RecvResponse (SETUP, 0) != 0) 
					return 31;
			}
		}
		else {
			for (i=1; i<=numMedia; i++) {
				if (SendRequest(SETUP, i) != 0) {
					MessageBox(NULL, "Failed sending SETUP request!", "Protocol error", MB_OK);
					return 21;
				}
				else{
					if (RecvResponse (SETUP, i) != 0) 
						return 31;
				}
			}
		}

		// Now it's just to start media tools.

		for (i=1; i<=numMedia; i++) {
			sprintf (str, " %s/%d", media[i].srcAddr, media[i].uniPort);

			//接收数据启动
			m_iRtspDataPort = media[i].uniPort;
			//rtcp的端口
			//启动接收RTCP的线程
			m_iRtcpPort = m_iRtspDataPort+1;
			HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, Rtcp_Data_recv, (LPVOID)this, 0, &m_RtcpThreadID);
			CloseHandle (ThreadHandle);
		}
/*	
#ifdef WIN32
		if (stricmp (media[i].type, "video") == 0) {
				sprintf (command, "%s -C \"%s%s\"", theApp.videoAppPath, "Video: ", sessName);
			}
			else if (stricmp (media[i].type, "audio") == 0) {
				strcpy (command, theApp.audioAppPath);
			}
			else if (stricmp (media[i].type, "text") == 0) {
				strcpy (command, theApp.textAppPath);
			}
#endif
#ifdef UNIX
			if (stricmp (media[i].type, "video") == 0) {
				sprintf (command, "vic -C \"Video: %s\"", sessName);
			}
			else if (stricmp (media[i].type, "audio") == 0) {
				strcpy (command, "rat");
			}
			else if (stricmp (media[i].type, "text") == 0) {
				strcpy (command, "nte");
			}
#endif

			else {
				char msg[MAX_STR];

				strcpy (msg, "Unknown media type ");
				strcat (msg, media[i].setupInfo);
				MessageBox (NULL, msg, "WARNING!", MB_OK);
				break;
			}
			strcat (command, str);
			
#ifdef WIN32
			STARTUPINFO si;
		    ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			
			if( !CreateProcess( NULL, // No module name (use command line). 
					command,		  // Command line. 
					NULL,             // Process handle not inheritable. 
					NULL,             // Thread handle not inheritable. 
					FALSE,            // Set handle inheritance to FALSE. 
					0,                // No creation flags. 
					NULL,             // Use parent's environment block. 
					NULL,             // Use parent's starting directory. 
					&si,              // Pointer to STARTUPINFO structure.
					&mph[i] )             // Pointer to PROCESS_INFORMATION structure.
				) 
			{	
				MessageBox (NULL, "Could not start " + (CString)command +
					".\nCheck PATH environment variable!", "WARNING!", MB_OK);
				mph[i].hProcess = NULL;
			}
#endif
#ifdef UNIX
			strcat (command, " &");
			system (command);
#endif
		}
	*/
		// .... and play!
		if (SendRequest (PLAY, 0) != 0) {
			MessageBox(NULL, "PLAY request failed!", "Protocol Error", MB_OK);
			return 22;
		}
		else {
			if (RecvResponse (PLAY, 0) != 0) 
				return 32;
		}

	}

	state = PLAYING;
	return res;
}

int CRTSPclient::Pause (){
	int res =0;

	if (state == PLAYING) {
		// we are playing
		if (SendRequest (PAUSE, 0) == 0) {
			RecvResponse (PAUSE, 0);
			state = PAUSED;
		}
		else {
			res =1;
		}
	}

	return res;
}

int CRTSPclient::Resume (){
	int res =0;

	if (state == PAUSED) {
		// we are in pause
		if (SendRequest (PLAY, 0) == 0) {
			RecvResponse (PLAY, 0);
			state = PLAYING;
		}
		else
			res =1;
	}
	return 0;
}

int CRTSPclient::Disconnect (){

	m_bstop = false;
	if (SendRequest (TEARDOWN, 0) == 0) {
		RecvResponse (TEARDOWN, 0);
	}
	state = IDLE;
	strcpy (session, ""); 
	strcpy (sessName, ""); 
	strcpy (accept, "");	
	numMedia = 0;	

	if (serv_socket)
#ifdef WINS
		closesocket (serv_socket);
#else
//		close (serv_socket);
#endif

	/* We need MBus !!!!!!!!!! Forget this:
	for (i=1; i<=numMedia; i++) 
		if (mph[i].hProcess > 0) {
		  // Close process and thread handles for media tools
			CloseHandle(mph[i].hProcess);
			CloseHandle(mph[i].hThread);
		}
	*/

	return 0;
}

const char* CRTSPclient::GetTitle () {
	return sessName;
}

int CRTSPclient::ParseInput (const char * serverURL) {
	int res = 0;
	//int p,q,r;
	char *s1, *s2, *s3;  // host, port, file
	char st[MAX_STR];

	s1 = strstr (serverURL, "//");
	if (s1 == NULL) 
		s1 = (char *)serverURL; //
	else {
		s1+=2;
		if (strnicmp (serverURL, "RTSP", 4) != 0)
			return 1; // Bad protocol
	}

	s3 = strchr (s1, '/'); 
	if (s3 == NULL)
		return 2; // No file specified, there's no default like in HTTP
	
	s2 = strchr (s1, ':');
	if (s2 == NULL) {
		port = RTSP_SEVER_PORT;
		strncpy (host, s1, (size_t)(s3-s1));
	} else {
		strncpy (st, s2+1, (size_t)(s3-s2-1));
		port = atoi (st);
		strncpy (host, s1, (size_t)(s2-s1));
		host[s2-s1] ='\0';
	}

	strcpy (file, s3+1);
	return res;
}


int CRTSPclient::GetSomePort (){
	int candidate = 15000 + (lastPort+2) % 15000; // so use ports 15000-30000
	SOCKET tmp_socket;

	int socket_type = SOCK_STREAM;
	struct sockaddr_in server;
	struct hostent *hp;
	bool sucess = 0;

	hp = gethostbyname("localhost");
	if (hp == NULL ) {
		MessageBox(NULL, "I do not know who 'localhost' is\n-- please setup your network configuration :)", "ERROR", MB_OK);
		return -1;
	}

	while (!sucess) {
		memset(&server,0,sizeof(server));
		memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
		server.sin_family = hp->h_addrtype;
		server.sin_port = htons(candidate);

		if ((tmp_socket = socket(AF_INET,socket_type,0)) == SOCKET_ERROR) 
			candidate+=2;
		else {
			if (bind (tmp_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) 
				candidate+=2;
			else 
				sucess = 1;
#ifdef WINS
			closesocket (tmp_socket);
#else
//			close (tmp_socket);
#endif
		}
	}

	lastPort = candidate;
	return candidate;
}

////////////////////////////////////
//
// SendRequest:
// type: DSCRIBE, PLAY, etc.
// media: 0<=media<=MAX_MEDIA. '0' is 'all media'. Ignored in DESCRIBE req.type

int CRTSPclient::SendRequest (int type, int mediaCtrl) { 
	int res = 0;
	int len, cur =0;
	char nl[3] = "\r\n";
	char reqString[MAX_STR];
	char *send_p;
	char buf[20];
	char seqNum[MAX_STR];

	switch (type) {
		case PLAY: {
			strcpy (reqString, "PLAY ");
			break;
		}
		case DESCRIBE: {
			strcpy (reqString, "DESCRIBE ");
			strcpy (accept, "application/sdp");
			break;
		}
		case SETUP: {
			strcpy (reqString, "SETUP ");
			break;
		}
		case PAUSE: {
			strcpy (reqString, "PAUSE ");
			break;
		}
		case TEARDOWN: {
			strcpy (reqString, "TEARDOWN ");
			break;
		}
	}

	if ((mediaCtrl <0) || (mediaCtrl > numMedia))
		return 2;
	
	strcat (reqString, "rtsp://");
	strcat (reqString, host);
	strcat (reqString,":");
	char txttmp[128]={0};
	sprintf(txttmp,"%d",port);
	strcat (reqString,txttmp);
	strcat (reqString, "/");
	strcat (reqString, file);
	if (type == SETUP)
		strcat (reqString, media[mediaCtrl].setupInfo);
	strcat (reqString, " RTSP/1.0");
	strcat (reqString, nl);

	sprintf (buf, "%d", ++cSeq); // itoa was here
	strcpy (seqNum, buf);
	strcat (reqString, "CSeq: ");
	strcat (reqString, seqNum);
	strcat (reqString, nl);
	if ((strlen (accept) > 0) && (type == DESCRIBE)) {
		strcat (reqString, "Accept: ");
		strcat (reqString, accept);
		strcat (reqString, nl);
	}

	/*
		play 快进 
		RTSP/1.0 200 OK
		CSeq: 497
		Date: Mon, May 26 2014 13:27:07 GMT
		Range: npt=0.000-   //快进范围
		Session: 151
		RTP-Info: url=rtsp://192.168.103.51:8552/h264_ch2/track1;seq=63842;rtptime=1242931431,url=rtsp://192.168.103.51:8552/h264_ch2/track2;seq=432;rtptime=3179210581

	*/
	if ((strlen (session) > 0) && (type != DESCRIBE)) {
		printf("%s",session);
		strcat (reqString, "Session: ");
		strcat (reqString, session);
		strcat (reqString, nl);
	}
	if (type == SETUP) {
		strcat (reqString, "Transport: RTP/AVP;unicast");
		if (mediaCtrl != 0) {
			char str[13];
			sprintf (str, "%d-%d", media[mediaCtrl].uniPort, media[mediaCtrl].uniPort+1);
			strcat (reqString, ";client_port=");
			strcat (reqString, str);
		}
		strcat (reqString, nl);
	}

	strcat (reqString, nl);
	strcat (reqString, "\0");

	len = strlen (reqString);
	send_p = reqString;
	while (len>0)
		if ((cur = send (serv_socket, send_p, len-cur, 0)) == SOCKET_ERROR) {
			res = 1;
			break;
		}
		else {
			send_p += cur; 
			len -= cur;
		}
  
	return res;
}

///////////////////////////////////////
// RecvResponse

int CRTSPclient::RecvResponse (int type, int mediaCtrl) {
	int res = 0;
	int len, p, cur =0;
	char buf[MAX_BUF];
	char line[MAX_LINE];
	char str[MAX_LINE];  // It can't be longer than the line :)
	char resp[MAX_RESP_SIZE]; // TODO: How long it can be?
	int resp_len, resp_cur;
	int bodyLen = 0;
	bool moreToRead;
	int mediaNo = -1;


	ZeroMemory (buf, MAX_BUF);
	ZeroMemory (resp, MAX_RESP_SIZE);

	// Receive header
	if ((len = recv (serv_socket, buf, MAX_BUF, 0)) != SOCKET_ERROR) {
		strncpy (resp, buf, len);
		resp_len = len;
		resp_cur = 0;
	}
	else {
		res = 101;
		goto done;
	}	

	// Understand status line
	if ((len = readLine (resp, cur, resp_len, line)) != -1) {
		cur =0;
		while ((cur < len) && (line[cur++] != ' '));
		if (line[cur] != '2') {
			// We have non 2xx response, fail
			if (resp_len>256)
				resp[255] = '\0';
			MessageBox (NULL, resp, "Server Error", MB_OK);
			res = 110;
			goto done;
		}
		resp_cur = len;
	}

	moreToRead = TRUE; // Now we have to understand different header lines
	while (moreToRead) {
		while ((len = readLine (resp, resp_cur, resp_len, line)) >= 2) { // All text lines at least 2
			resp_cur += len;
			if (len == 2) {
				// No more header
				moreToRead = FALSE;
				break;
			}
			else {
				// Obtain the first string			
				cur =0;
				while ((cur < len) && (line[cur] != ' '))
					str[cur] = line[cur++];
				if (strnicmp (str, "Content-Length:", 15) == 0) {
					// Obtain the second string			
					p = 0;
					while ((++cur < len) && (isdigit (line[cur])))
						str[p++] = line[cur];
					sscanf (str, "%d", &bodyLen);
				}
				else if (strnicmp (str, "CSeq:", 5) == 0) {
					p = 0;
					while ((++cur < len) && (isdigit (line[cur])))
						str[p++] = line[cur];
					str[p] = 0;
					if (cSeq != atoi(str))
						MessageBox (NULL, "Bad Sequence Number in Response!", "Warning", MB_OK);
				}
				else if (strnicmp (str, "Session:", 8) == 0) {
					p = 0;
					strcpy (session, "");
					while ((++cur < len) && (isgraph (line[cur])) && (line[cur] != ';'))
						session[p++] = line[cur];
				}
				else if (strnicmp (str, "Transport:", 10) == 0) {
					char *lineS, *lp = 0;

					lineS = strstr (line, "source=");
					if (lineS != NULL) 
						lineS = strchr (lineS, '=');
					if (lineS != NULL) 
						lp = strchr (lineS, ';');
					if ((lp - lineS)< MAX_STR) {
						p = lp - lineS -1; // String lentgth to be copied
						strncpy (media[mediaCtrl].srcAddr, lineS+1, p);
						media[mediaCtrl].srcAddr[p] = '\0'; // This is not allways appended automatically
						memset(m_strRTPServerIP,0,sizeof(m_strRTPServerIP));
						//strncpy (m_strRTPServerIP, lineS+1, p);
						strcpy(m_strRTPServerIP,media[mediaCtrl].srcAddr);
						printf("----%s \n",m_strRTPServerIP);
					}
					char *lineportend,*lineServerPort =NULL;


					lineServerPort = strstr(line,"client_port=");
					if(lineServerPort != NULL)
					{
						lineServerPort = strstr(lineServerPort,"=");

					}
					if(lineServerPort != NULL)
					{
						lineportend = strstr(lineServerPort,"-");
					}
					if ((lineportend - lineServerPort)< MAX_STR) {
						p = lineportend - lineServerPort -1; // String lentgth to be copied
						char strServerPort[128] = {0};
						strncpy (strServerPort, lineServerPort+1, p);
						strServerPort[p]='\0';
						m_iRtspDataPort = atoi(strServerPort);
					}

					lineportend=NULL;
					lineServerPort =NULL;

					lineServerPort = strstr(line,"server_port=");
					if(lineServerPort != NULL)
					{
						lineServerPort = strstr(lineServerPort,"=");

					}
					if(lineServerPort != NULL)
					{
						lineportend = strstr(lineServerPort,"-");
					}
					if ((lineportend - lineServerPort)< MAX_STR) {
						p = lineportend - lineServerPort -1; // String lentgth to be copied
						char strServerPort[128] = {0};
						strncpy (strServerPort, lineServerPort+1, p);
						strServerPort[p]='\0';
						m_iRTPServerPort = atoi(strServerPort);
					}
					
					TestRTPLinkRight();
				}
			}
		}
		if (len == -1) {
			// Try to read more data
			if ((len = recv (serv_socket, buf, MAX_BUF, 0)) != SOCKET_ERROR) {
				strncpy ((char *)(resp+resp_len), buf, len);
				resp_len += len;
			}
			else {
				res = 101;
				goto done;
			}	
		}
	}


	/* Read the body, this should be SDP syntax.  We have read resp_len from the
		socket, and resp_cur is the header. Contrary to the header, we know how much
		we have to read now: bodyLen bytes */

	while (resp_len < resp_cur + bodyLen) {
		if ((len = recv (serv_socket, buf, MAX_BUF, 0)) != SOCKET_ERROR) {
			strncpy ((char *)(resp+resp_len), buf, len);
			resp_len += len;
		}
		else {
			res = 101;
			goto done;
		}	
	}


	if (type == DESCRIBE) {
		// Now we are finished reading from the socket. Process the SDP!

		mediaNo = 0;
		while ((len = readLine (resp, resp_cur, resp_len, line)) >= 0) {
			resp_cur += len;
			switch (line[0]) {
				case 's': {
					// Session name
					strcpy (sessName, line+2);
					break;
				};
				case 'm': {
					if (++mediaNo >= MAX_MEDIA) {
						--mediaNo;
						break;
					}
					cur = 2; p=0;
					strcpy (media[mediaNo].type, "");
					while (isgraph (line[cur]))
						media[mediaNo].type[p++] = line[cur++];
					p=0; cur++;
					while (isdigit (line[cur]))
						str[p++] = line[cur++];
					str[p] = '\0';
					media[mediaNo].multiPort = atoi (str);

					media[mediaNo].uniPort = GetSomePort();
					break;
				}
				case 'c': {
					cur = 2;
					// We start after the second space, so pass nettype (we guess Internet) and pass addrtype (we guess IPv4)
					while ((line[++cur] != ' ') && (cur < len));
					while ((line[++cur] != ' ') && (cur < len));

					cur++; p=0;
					while (isgraph (line[cur]) && (line[cur] != '/'))
						media[mediaNo].srcAddr[p++] = line[cur++];
					if (line[cur] == '/') {
						p=0; cur++;
						while (isdigit (line[cur]))
							str[p++] = line[cur++];
						str[p] = '\0';
						media[mediaNo].multiTTL = atoi (str);
					}
					break;
						  }
				case 'a': {
					if (strnicmp ("a=control:", line, 10) == 0) {
						cur = 10;
						p=1;
						str[0]='/';
						while (isgraph (line[cur]))
							str[p++] = line[cur++];
						str[p] = '\0';
						strcpy (media[mediaNo].setupInfo, str);
					}
					break;
				}
				default:
				;					  
			}
		}
		numMedia = mediaNo;
	}
/*	else if (type == SETUP)
	{
		
		while ((len = readLine (resp, resp_cur, resp_len, line)) >= 0) {
			resp_cur += len;

		}
	}
*/
	else {
		// this was not a DESCRIBE response
#ifdef WIN32
		ASSERT (resp_len == resp_cur);
#endif
	}

done:
	return res;
}

int  CRTSPclient::TestRTPLinkRight()
{
	unsigned int addr;
	int socket_type = SOCK_DGRAM;
	struct sockaddr_in server;
	SOCKET serv_socket_rtp;

	struct sockaddr_in c_addr; 
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(m_iRTPServerPort);
	int iport = m_iRTPServerPort;
	c_addr.sin_addr.s_addr = inet_addr(m_strRTPServerIP);
	char ttmp[128]={0};
	strcpy(ttmp,m_strRTPServerIP);

	memset(&server,0,sizeof(server));
	//memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
	server.sin_family = AF_INET;
	server.sin_port = htons(m_iRtspDataPort);
	server.sin_addr.s_addr = ADDR_ANY;//inet_addr(host);

	serv_socket_rtp = socket(AF_INET,socket_type,0); /* Open the socket */
	int nRecvBuf = 0;
	int iLen = 4;
	setsockopt ( serv_socket_rtp, SOL_SOCKET, SO_REUSEADDR, (char*)&nRecvBuf, sizeof(int) );

	if ( (bind(serv_socket_rtp, (struct sockaddr*)&server, sizeof(server))) == -1 ) {
			//perror("bind");
			//exit(errno);
		printf("bind address to socket.\n\r");
	}else
		printf("bind address to socket.\n\r");
	char RTP_TestBuff[128]={0};
	RTP_TestBuff[0]=0xce;
	RTP_TestBuff[1]=0xfa;
	RTP_TestBuff[2]=0xed;
	RTP_TestBuff[3]=0xfe;
	
	int ilen = sendto(serv_socket_rtp,RTP_TestBuff,strlen(RTP_TestBuff),0,(struct sockaddr*)&c_addr,sizeof(c_addr));
	if(ilen <=0)
	{
		printf("bind address to socket.\n\r");
	}
	ilen = sendto(serv_socket_rtp,RTP_TestBuff,strlen(RTP_TestBuff),0,(struct sockaddr*)&c_addr,sizeof(c_addr));
	if(ilen <=0)
	{
		printf("bind address to socket.\n\r");
	}
	closesocket(serv_socket_rtp);
	return 0;
}

int CRTSPclient::readLine (char* from_str, int first, int max, char* to_str) {
	int res =0;
	int count = 0;

	// We support "\r\n" and "\n" line termination
	while ((first + count < max) && (count < MAX_LINE)) {
		if (from_str[first+count] == '\n') { 
			to_str[count] = '\0';
			count+=1;
			res = count;
			break;
		}
		if (from_str[first+count] == '\r') { 
			to_str[count] = '\0';
			count+=2;
			res = count;
			break;
		}

		to_str[count] = from_str[first+count];
		count++;
	}

	if (res == 0) {
		if (first + count == max)
			res = -1; // from_str is empty, read more from e.g. the socket
		else {
			MessageBox(NULL, "Danger: very long lines received from the server!", NULL, MB_OK);
			res = -2;
		}
	}

	return res;
}


int CRTSPclient::GetRtspPort()
{
	
	return m_iRtspDataPort;
}

int CRTSPclient::GetRtpServerPort()
{

	return m_iRTPServerPort;
}