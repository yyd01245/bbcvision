#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#pragma comment(lib,"Ws2_32.lib")
int main()
{
	WSADATA wsaData;
	int Ret;
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("WSAStartup Failed \n");
		return -2;
	}

	short sTCPServerPort = 6869;
	
	SOCKET sockTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in TCPaddr;
	TCPaddr.sin_family = AF_INET;
	TCPaddr.sin_port = htons(sTCPServerPort);
	TCPaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int ret = 0;
	SOCKET socktm;
	ret = connect(sockTCP, (sockaddr*)&TCPaddr, sizeof(TCPaddr));
	if(ret==0)
	{
		printf("connect success\n");
		//string str = "{ "cmd":"streamstart", "iip" : "192.168.100.106", "iport" : "6899", "rate" : "375000", "url" : " / usr / ts / a.ts", "serialno" : "db974e9d8f424d2da87b246831442867" }";
		send(sockTCP, "1", 1, 0);
		Sleep(1000*30);
		send(sockTCP, "2", 1, 0);
		Sleep(1000 * 5);
		send(sockTCP, "2", 1, 0);
		Sleep(1000 * 30);
		send(sockTCP, "3", 1, 0);
	}
	Sleep(100000);
}
