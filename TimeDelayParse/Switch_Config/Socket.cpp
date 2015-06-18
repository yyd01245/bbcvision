
/**************************************************************************
 * \file    rate_sock.cpp
 * \brief   
 *
 * Copyright (c) 2006 OnewaveInc.
 * RCS: $Id: rate_sock.cpp,v 1.9 2008/08/06 10:39:17 chenjx Exp $
 *
 * History
 *  2006/10/11 leiq     created
**************************************************************************/


#include "Socket.h"

//------------------------------Socket-----------------------------------
int Socket::read(char* pBuf, int p_iMax_Len,int p_iMin_Len,int p_itime_out)
{
	int		  iLeft = p_iMax_Len,i_timeleft=p_itime_out;
	time_t  t_start = time(NULL);
	int     i;
	while( iLeft > 0 )
	{
		if(!is_Valid()) 
		{
    	 	  //cout<<"socket is not Valid : "<<m_nSock<<endl;
    	 	  return -1;
    }
		
		for(;i_timeleft >0;) 
		{ 
			fd_set events;
			struct timeval tm;
			FD_ZERO(&events);
			FD_SET(m_nSock, &events);
			tm.tv_sec = i_timeleft;
			tm.tv_usec = 0;
			while ((i = select(m_nSock+1, &events, NULL, NULL, &tm)) < 0 
				&& errno == EINTR);
			if( i < 0 ) 
			{
    	 	  //cout<<"select return "<<i<<endl;
    	 	  return -1;
    	}
			i_timeleft = t_start+p_itime_out - time(NULL);
			if(FD_ISSET(m_nSock,&events)) break;
			//if(i==0&&is_Bad()) return -1;
		};
		
		i = ::recv(m_nSock, pBuf, iLeft,0);
		if( i < 0 )     
		{
			cout<<"recv return "<<i<<endl;
			perror("recv:");
			return	-1;
		}
		iLeft -= i;
		pBuf += i;
		//printf("i_timeleft = %d \n",i_timeleft);
		if((p_iMax_Len - iLeft >= p_iMin_Len) || (i_timeleft <= 0)) break;

	}
	return ( p_iMax_Len - iLeft);
}


int Socket::write( const char* pBuf, int nLen,int p_itime_out)
{
	int	  iLeft = nLen,i_timeleft=p_itime_out;
  time_t  t_start = time(NULL);
	while( iLeft > 0 )
	{
		int	i = ::send( m_nSock, pBuf, iLeft,0);
		if( i <= 0 )   return -1;
		iLeft -= i;
		pBuf += i;
		if(iLeft <=0|| i_timeleft<=0) break;
					
		do
		{ 
			fd_set events;
			struct timeval tm;
			FD_ZERO(&events);
			FD_SET(m_nSock, &events);
			tm.tv_sec = i_timeleft;
			tm.tv_usec = 0;
			while ((i = select(m_nSock+1, NULL,&events, NULL, &tm)) < 0 
				&& errno == EINTR);
			if( i < 0 ) return	-1;
			if(FD_ISSET(m_nSock,&events)) break;
			i_timeleft = t_start+p_itime_out - time(NULL);
			if(i==0&&is_Bad()) 	return -1;
			}
		while (i_timeleft >0) ;
	}
	return ( nLen - iLeft);
}

int Socket::readyLength()
{
	  unsigned char buf[32768+1];

    int bytes;
    if(m_nSock <0) return -1;

    bytes=::recv(m_nSock, (char *)buf, 32768,MSG_PEEK);
    if(bytes<0)  bytes=32768;

    return bytes;
}

void Socket::close()
{
    if(m_nSock>0)
    {
        ::close(m_nSock);
        m_nSock=-1;
    }
}

bool Socket::is_Bad()
{
	struct stat buf;
	fstat(m_nSock, &buf);
	return errno == EBADF;	
}

bool Socket::get_peer(string &host,int &port)
{
    struct sockaddr sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		socklen_t i_len = sizeof(sockAddr);
	
    if(getpeername(m_nSock,&sockAddr, &i_len)<0)
     return false;
    
    struct sockaddr_in *addr= (sockaddr_in *)&sockAddr;
    host=inet_ntoa(addr->sin_addr);
    port=ntohs(addr->sin_port);	
    return true;
}

//------------------------------Socket-----------------------------------



//------------------------------Socket_Connector-------------------------

bool Socket_Connector::connect(Socket &new_sock,const char* p_host,int p_Port)
{
    struct hostent *hp;
    struct sockaddr_in sin;
    int    sockid,isaddr=1;
    char * pos;
    
    sin.sin_family = AF_INET ;
    pos = (char *)p_host;
    while(*pos!=0&&*pos!=0x0d&&*pos!=0x0a)
    {
      if((*pos<='9'&&*pos>='0')||*pos==' '||*pos=='.')
      {
        pos++;
        continue;
      };
      isaddr=0;
      break;
    };
    if(isaddr)
      sin.sin_addr.s_addr = inet_addr(p_host) ;
    else
    {
      if((hp=gethostbyname(p_host))==NULL)
      {
         //cout<<"ERROR: gethostbyname"<<p_host<<endl;
         return false;
      }
      else
        memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    };
    
    sockid=socket(AF_INET,SOCK_STREAM,0);
    sin.sin_port=htons((unsigned short)p_Port);
    fcntl(sockid,F_SETFD, FD_CLOEXEC);
    //重复连接3次，防止中断等原因导致的异常
    for(int i=0;i<3;i++)
    {
    	 if(0 == ::connect(sockid,(struct sockaddr *)&sin,(int)(sizeof(sin)))) break;
    	 if(i==2) 
    	 {
    	 	  //cout<<"connect Error "<<p_host<<":"<<p_Port<<endl;
    	 	  //perror("::connect");
    	 	  ::close(sockid);
    	 	  return false;
    	 }
    	 Pubc::uSleep(100);
    	 //cout<<"connect again: "<<p_host<<":"<<p_Port<<endl;
    }
    
    int     optval = 1;
    setsockopt(sockid,SOL_SOCKET,SO_KEEPALIVE,(char *)(&optval),sizeof(optval));
   
    new_sock.setHandle(sockid);
    //cout<<"debug: connect sockid = "<< sockid <<" :"<<new_sock.getHandle()<<endl;
    return true;
}

//------------------------------Socket_Connector-------------------------


//------------------------------Socket_Acceptor-------------------------

bool  Socket_Acceptor::open(int nPort)
{
	
	if ( nPort <=0 ) return false;
	int nSock = ::socket( AF_INET, SOCK_STREAM, 0);
	if( -1 == nSock )
	{
		perror("create socket error");
		return false;
	}

	struct sockaddr_in	sin;
	sin.sin_family		= AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port		= htons( nPort);
    
  	int     optval = 1;
 	setsockopt(nSock,SOL_SOCKET,SO_KEEPALIVE,(char *)(&optval),sizeof(optval));
  	setsockopt(nSock,SOL_SOCKET,SO_REUSEADDR,(char *)(&optval),sizeof(optval));
/*
	struct linger lgr;
	lgr.l_onoff=1;
	lgr.l_linger=0;
	setsockopt( nSock, SOL_SOCKET , SO_LINGER , &lgr , sizeof(lgr) );
*/
	fcntl(nSock,F_SETFD, FD_CLOEXEC);
	if(::bind( nSock, (struct sockaddr*)&sin, sizeof(sin)) < 0 )
	    return false;
	::listen( nSock,  BACKLOG);
	m_nSock = nSock ;
	return true;
}

bool  Socket_Acceptor::accept(Socket &new_sock,int timeout_sec)
{
	struct	sockaddr_in	clientAddr;
	socklen_t	nClientLen = sizeof(sockaddr_in);
	int			iSock=0;

  if(m_nSock<=0) return false;
  
	if (timeout_sec>0)
	{
			time_t  t_start = time(NULL);
			int     i,i_timeleft=timeout_sec;
			do
		  { 
				fd_set events;
				struct timeval tm;
				FD_ZERO(&events);
				FD_SET(m_nSock, &events);
				tm.tv_sec = i_timeleft;
				tm.tv_usec = 0;
				while ((i = select(m_nSock+1, &events, NULL, NULL, &tm)) < 0 
					&& errno == EINTR);
				if( i < 0 ) return	false;
				if(FD_ISSET(m_nSock,&events)) break;
				i_timeleft = t_start+timeout_sec - time(NULL);
				if(i==0&&i_timeleft<= 0) return false;
				}while (i_timeleft >0) ;
	}
	iSock = ::accept(m_nSock, (struct sockaddr*)&clientAddr, &nClientLen);
  if(iSock<=0) return false;
  fcntl(iSock,F_SETFD, FD_CLOEXEC);
  new_sock.setHandle(iSock); 
	return true;
}

void  Socket_Acceptor::close()
{
	  if(m_nSock>0)
    {
        ::close(m_nSock);
        m_nSock=-1;
    }
}

//------------------------------Socket_Acceptor-------------------------

//------------------------------Socket_Server_TPC-------------------------

Socket_Server_TPC::Socket_Server_TPC()
{
		m_bExit=false;
		m_bListened = false;
		m_Port =0;
}

int Socket_Server_TPC::run()
{
		int sock_id=0;
		
		if(!m_bListened) 
			{
				m_bListened=true;
				return accept();
			}
			
		if(m_bListened)
		{
		   if(m_queueSock.size()==0) return 0;
			 m_mutex.lock();
			 sock_id=m_queueSock.front();
			 m_queueSock.pop();
			 m_mutex.unlock();
		}
		
		if(sock_id>0)
			 return worker(sock_id);
		else
			 return 0;					 
}

int Socket_Server_TPC::accept()
{
  	if (m_Port<=0) return -1;
  	Socket_Acceptor acceptor;
  	if(!acceptor.open(m_Port))
		{
			 cout<<"open error"<<endl;
			 return -1;
		}
		Socket sock;
		string cli_host;
		int    cli_port;
				 
		while(m_bExit==false)
		{
				if(acceptor.accept(sock,1))//非阻塞，一秒超时一次 
				{
					 //if(sock.get_peer(cli_host,cli_port))
			 	   		//cout<<"new connection: id ="<<sock.getHandle()<<" host ="<< cli_host <<" port="<<cli_port <<endl;
			 	   int s_id=sock.getHandle();
			 	   m_queueSock.push(s_id);
		 			 start(1);//启动一个处理线程
		 	 }
		 }	
		 return 0;
}

int  Socket_Server_TPC::worker(int socket_id)
{
	 Socket sock(socket_id);
	 char buf[40960];
	 int rc=0,i_err_count=0,i_readed=0;
	 string s_read,s_write;
	 //cout<<"thread "<<pthread_self()<<" entering ,sock_id= "<< socket_id <<endl;
	 while(m_bExit==false)
	 {
	 	  fd_set events;
			struct timeval tm;
			FD_ZERO(&events);
			FD_SET(sock.getHandle(), &events);
			tm.tv_sec = 1;
			tm.tv_usec = 0;
			while ((rc = select(sock.getHandle()+1, &events, NULL, NULL, &tm)) < 0 
				&& errno == EINTR);
		  if(rc==0) continue;
	 	  rc=sock.read(buf+i_readed,40960 - i_readed,1,1);//最少读取一个字节，超时1秒
	 	  if(rc<=0&& ++i_err_count > 3) break;
	 		if(rc>0) 
	 		{
	 			 i_readed+=rc;
	 			 buf[i_readed]='\0';
	 			 i_err_count=0;
	 			 s_read  = buf;
	 			 s_write ="";
	 			 rc = process(s_read,s_write);
	 			 if(rc < 0) break;
	 			 if(rc > 0) continue;//继续读
	 			 i_readed=0;
	 			 
	 			 if(s_write.size()>0)
	 			 	  rc=sock.write(s_write.c_str(),s_write.size());
	 			 if(rc < 0) break;
		 	}

	 }
	 sock.close();
	 //cout<<"thread "<<pthread_self()<<" exiting " <<endl;
	 return 0;
}
		
//------------------------------Socket_Server_TPC-------------------------

