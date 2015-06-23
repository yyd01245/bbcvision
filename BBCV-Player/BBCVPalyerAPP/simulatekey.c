#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

//#define UDP_IP  "21.254.236.252"
//#define UDP_IP  "192.168.200.202"
#define UDP_IP  "192.168.70.127"
#define PORT_START 62000

int g_number;

typedef struct {
        unsigned int dev_type;
} BlcYunKeyMsgHead;

typedef struct {
        unsigned int sequence_num;
        unsigned int key_value; 
        unsigned int key_status; 

} BlcYunKeyIrrMsgBody;

typedef struct {
    BlcYunKeyMsgHead head;
        BlcYunKeyIrrMsgBody body;
} BlcYunKeyIrrMsg;
#if 0
int get_key()
{
        static int i = 1;
        i++;
        if(i>2)
                i=1;
        switch(i)
        {
            case 1: return 82;//up
             case 2: return 81;//down
          //   case 1: return 79;//left
        	//     case 2: return 80;//right
        	//      case 1: return 40;//enter
        	//      case 2: return 158;//return (Insert)
                default : return -1;
        }
}
#else
int get_key()
{
        static int i = 0;
        i++;

        if(i%18<9)
            return 79;
        else
            return 80;

}



#endif




int main(int argc, char **argv)
{
        g_number = 60;
        struct sockaddr_in s_addr;
        int sock;
        int addr_len;
        int len,i;
        char buff[128];
        int port;
        int key;

        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
                perror("socket");
                exit(errno);
        }

        BlcYunKeyIrrMsg irrmsg;
        while(1)
        {
                key = get_key();
                port = PORT_START;
                for(i=1;i<=g_number;i++)
                {
                        s_addr.sin_family = AF_INET;
                        s_addr.sin_addr.s_addr = inet_addr(UDP_IP);
                        s_addr.sin_port = htons(port+i);
                        addr_len = sizeof(s_addr);
                        irrmsg.head.dev_type = 1001;
                        irrmsg.body.key_value = key;
                        irrmsg.body.key_status = 2;
                        len = sendto(sock, (char*)&irrmsg, sizeof(BlcYunKeyIrrMsg), 0,(struct sockaddr *) &s_addr, addr_len);
                        printf("send len:%d port:%d key : %d\n",len,port+i,irrmsg.body.key_value);
                        irrmsg.body.key_status = 3;
                        len = sendto(sock, (char*)&irrmsg, sizeof(BlcYunKeyIrrMsg), 0,(struct sockaddr *) &s_addr, addr_len);
//                        port++;
                        usleep(50);
                }
                printf("send once\n");
                sleep(2);
        }
        return 0;
}
