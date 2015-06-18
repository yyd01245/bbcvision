#ifndef		_RATE_SOCK_H_
#define		_RATE_SOCK_H_

/**************************************************************************
 * \file    socket.h
 * \brief   
 *
 * Copyright (c) .
 * RCS: $Id: rate_sock.h,v 1.9 2008/08/06 10:39:17 leiq Exp $
 *
 * History
 *  2006/10/11 leiq     created
**************************************************************************/


#ifdef WINDOW_SYS
    #include <windows.h>
    #include <winbase.h>
#else
    #include	<netdb.h>
    #include	<unistd.h>
    #include	<arpa/inet.h>
    #include	<sys/socket.h>
	#include	<fcntl.h>
#endif
#include <string>
#include <iostream>
#include <queue>

#include "Thread.h"

using namespace std;

#define  BACKLOG 128 //listen backlog

/**************************************************************************
 * class :Socket
  * brief :
         1、常见的socket操作
         2、注意Socket仅提供方法封装，并不做任何处理，例如出错的情况下，函数并不主动关闭连接
**************************************************************************/
class Socket
{
public:
	Socket():m_nSock(-1){};
	Socket( int nSock ) { setHandle( nSock );};
	Socket(Socket &sock){ setHandle(sock.getHandle());};
	~Socket(){};
public:
	void		setHandle(int nSock){m_nSock=nSock;};
	int			getHandle(){	return m_nSock;};
	//返回读取字节数，正常或超时均返回读取字节数。出错时返回-1，不主动关闭连接
	int	    read(char* pBuf, int p_iMax_Len,int p_iMin_Len=0,int usec_time_out=5);
	int	    write( const char* pBuf, int nLen,int timeout_sec=30);

	//返回缓冲区可读的字节数,效率比较差，不建议高频度调用
	int     readyLength();
	void 		close();

	bool    is_Valid(){return m_nSock>0;};
	bool    is_Bad();
	bool    get_peer(string &host,int &port);
	
	
	Socket  &operator = (Socket &sock)
	{
		if(this != &sock)
			setHandle(sock.getHandle());
		return *this;
	};
	
protected:
	int	    m_nSock;
};

/**************************************************************************
 * class :Socket_Connector
  * brief :
         1、客户端链接器,默认设置SO_KEEPALIVE为true
**************************************************************************/
class Socket_Connector
{
public:
  Socket_Connector(){};
  static bool   connect(Socket &new_sock,const char* host,int nPort);
};


/**************************************************************************
 * class :Socket_Acceptor
  * brief :
     1、链接接收器,如果timeout_sec=0则为阻塞模式,用于比较简单的场景
**************************************************************************/
class Socket_Acceptor
{
public:
	Socket_Acceptor():m_nSock(-1) {};
	bool 	    open(int nPort); 
	int				getHandle(){ return m_nSock;};
	bool		  accept(Socket &new_sock,int timeout_sec=0);
	void 			close();
private:
	int	    	m_nSock;
};

/*
**************************************************************************
 * class :Socket_Server_TPC
  * brief :
     1、最为简单的socket server模式，Thread per Connecttion
**************************************************************************
*/
class Socket_Server_TPC: public Thread_Base
{
public:
	Socket_Server_TPC();
protected:
	/*
	  逻辑处理函数，最大一次接收buf为40k(40960)
	  返回:
	   0  ok
	  -1  关闭连接，退出线程
	   1  继续读取数据,不清除data 
	*/
	virtual int      process(string &data,string &return_date)=0;
public:
	int     				 stop() {m_bExit=true;};
	void 						 setPort(int port){m_Port=port;};
protected:
	int							 run();
  virtual int      worker(int sock_id);
private:
	int              accept();
	int 						 m_Port;
	bool             m_bExit;
	bool						 m_bListened;
	queue<int>       m_queueSock;
};


#endif

