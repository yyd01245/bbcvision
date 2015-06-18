
/**************************************************************************
 * \file    Socket_Ractor_Imp.h
 * \brief   
 *
 * Copyright (c) .
 * RCS: $Id: rate_sock.h,v 1.9 2008/08/06 10:39:17 leiq Exp $
 *
 * History
 *  2006/10/11 leiq     created
**************************************************************************/


#include "Socket_Ractor.h"
#include "assert.h"
#include "DateTime.h"


//------------------------------Socket_Set-------------------------


Socket_Set::Socket_Set()
{
	 reset();
}

void Socket_Set::reset()
{
	 FD_ZERO(&m_set);
	 FD_ZERO(&m_SelectSet);
	 m_MaxHande=0;
	 m_size=0;
	 memset(&m_ArrayHandles,0,sizeof m_ArrayHandles);
}


inline fd_set * Socket_Set::getSelectSet()
{
	 if(m_size>0)
	 {
	 	 m_SelectSet=m_set;//此变量会被外部select破坏
	 	 return &m_SelectSet;
	 }
	 return NULL;
}

inline int Socket_Set::getMaxHande()
{
	 return m_MaxHande;
}


inline	bool	Socket_Set::isValidHandle(int handle)
{
	return handle> 0 && handle < SOCKET_MAXSIZE;
}

inline bool Socket_Set::setBit(int handle)
{
	 if(!isValidHandle(handle)) return false;
	 FD_SET(handle,&m_set);
	 
	 if(m_MaxHande < handle)  m_MaxHande=handle;
	 if(m_ArrayHandles[handle]<=0) //如果不是已有的handle
	 {
	 		m_size++;
	 		m_ArrayHandles[handle]=1;
	 }
	 return true;
}


inline bool Socket_Set::clrBit(int handle)
{
	 if(!isValidHandle(handle)) return false;
	 FD_CLR(handle,&m_set);
 
	 if(m_ArrayHandles[handle]>0)
	 {
	 	 m_ArrayHandles[handle]=0;
	 	 m_size--;
	 };

	 if(m_MaxHande == handle) //重新计算m_MaxHande
	 {
	 	 for(;m_MaxHande>0;m_MaxHande--)
	 	 {
	 	 	 if(m_ArrayHandles[m_MaxHande]>0) break;
	 		}
	 }
	 return true;
}

inline bool Socket_Set::isSet(int handle)
{
	 return FD_ISSET(handle,&m_SelectSet)>0;
}

//此函数必须在select之后调用
int Socket_Set::getReadyHandle(queue<int> &lstHandle)
{
	 int iCount=0;
	 if(m_size>0)
	 {
	 		for(int i=0;i<=m_MaxHande;i++)
	 	   if(m_ArrayHandles[i]==1 && isSet(i) )
	 	   {
	 	   	  lstHandle.push(i);
	 	   	  iCount++;
	 	   }	  
	 }
	 return iCount;
}

//------------------------------Socket_Set-------------------------


//------------------------------Socket_Selector-------------------------

int Socket_Selector::check_io_once(int p_Millisecond)
{
	 
	 int i_count=0,i_max_id=0;
	 
	 struct timeval tm;
	 tm.tv_sec = p_Millisecond/1000;
	 tm.tv_usec =(p_Millisecond%1000) * 1000;
 
	 i_max_id=m_set.getMaxHande(); 
	 //cout<<"i_max_id = "<<i_max_id<< "tid ="<<pthread_self()<<endl; 
	 if(i_max_id<=0)  return  0;
 	 
 	 
 	 ThreadLocker locker(m_mutex_select);//线程互斥
	 while ((i_count = select(i_max_id+1, 
	                          m_set.getSelectSet(),
	                          NULL,
	                          NULL, 
	                          &tm)) < 0 
				&& errno == EINTR) ;
	 
	 if(i_count > 0)
	 {
	 	  queue<int>  q;
	 		i_count=m_set.getReadyHandle(q);
		  while(q.size()>0)
			{
			 	  	m_lstHandles_Ready.push(q.front());
			 	  	remove_Handle(q.front());
			 	  	q.pop();
			}
	 }
	 return i_count;
};



bool Socket_Selector::pop_ReadyHandle(int &handle,int timeout_Millisecond)
{
	 ThreadLocker locker(m_mutex_ready);//线程互斥
	 //cout<<"in  pop_ReadyHandle = "<<timeout_Millisecond<<endl;
	 //return false;
	 while(m_lstHandles_Ready.size()==0)//存在handle后续加入队列情况，因此不能直接阻塞长时间
	 {
	 	  check_io_once(3);
	 	  if(m_lstHandles_Ready.size() > 0) break;
	 	  //m_cond_seted.wait(3);
	 	  //Pubc::uSleep
	 	  timeout_Millisecond-=3;
	 	  //cout<<"timeout_Millisecond = "<<timeout_Millisecond<<endl; 
	 	  if(timeout_Millisecond <=0) return false;
	 }
	 
	 handle = m_lstHandles_Ready.front();
	 m_lstHandles_Ready.pop();
	 return true;
}

bool Socket_Selector::register_Handle(int handle)
{
	 ThreadLocker locker(m_mutex_set);
	 bool ret= m_set.setBit(handle);
	 //m_cond_seted.signal();
	 return ret;
}

bool Socket_Selector::remove_Handle(int handle)
{
	 ThreadLocker locker(m_mutex_set);
	 return m_set.clrBit(handle);
}

//------------------------------Socket_Selector-------------------------

//------------------------------Socket_Svc_Handler-------------------------
void Socket_Svc_Handler::handle_close()
{
	 	  //cout<<"thread "<<pthread_self()<<",Socket exiting : "<< getHandle() <<endl;
	 	  close();
};		
		
int Socket_Svc_Handler::handle_connected()
{
	 	 	string cli_host;
			int    cli_port;
			//cout<<" client connected id ="<<getHandle()<<endl;
	 	  //if(get_peer(cli_host,cli_port))
	 	  	 //cout<<" client connected id ="<<getHandle()<<" host ="<< cli_host <<" port="<<cli_port <<endl;
      return 0;
};
//------------------------------Socket_Svc_Handler-------------------------


/**************************************************************************
 * class :Socket_Ractor_Imp
  * brief :
    1、Ractor同时启动IO检查线程和处理线程
**************************************************************************/

//------------------------------Socket_Ractor_Imp-------------------------

Socket_Ractor_Imp::Socket_Ractor_Imp()
{
	 setName("Ractor");
	 m_bExit=false;
}

		

Socket_Ractor_Imp::~Socket_Ractor_Imp()
{
	 map<int,Socket_Svc_Handler *>::iterator it;
	 for(it=m_mapHandlers.begin();it!=m_mapHandlers.end();it++)
	 {
	 	 delete it->second;
	 }
	 m_mapHandlers.clear();
}



bool Socket_Ractor_Imp::open(int port,int process_num)
{
	 assert(getChecker()!=NULL);
	 //启动处理线程
	 if(start(process_num)==false)  
	 {
	 	 cout<<"error : at  start"<<endl;
	 	 return false ;
	 }
	 //开启监听
   if(m_acceptor.open(port)==false) 
   {
	 	 cout<<"error : at  m_acceptor.open"<<endl;
	 	 return false ;
	 }
    
	 /*if(getChecker()->register_Handle(m_acceptor.getHandle())==false) 
	 {
	 	 cout<<"error : at  register_Handle"<<endl;
	 	 return false ;
	 }*/
	 
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr,1048576);
	pthread_t id = 0;
	if (pthread_create(&id, &attr, &accept_client, this) != 0) 
		return false;
	
	 return true;
}



int Socket_Ractor_Imp::stop()
{
	  m_bExit=true;
	  m_acceptor.close();
}

/*
bool Socket_Ractor_Imp::wait()
{
	  sleep(1);
	  wait();
	  return true;
}
*/

void * Socket_Ractor_Imp::accept_client(void* pParam)
{
	Socket_Ractor_Imp * pThread = static_cast <Socket_Ractor_Imp*> (pParam);
	int iHandle=0;
	Socket l_sock;
	while(1)
	{
		if(!pThread->m_acceptor.accept(l_sock,1)) 
			continue;
		iHandle=l_sock.getHandle();
		if(pThread->getChecker()->register_Handle(iHandle)==false)
			l_sock.close();//拒绝多余的连接
		if(iHandle>1000) 
			usleep(5);
			
	}
}

int Socket_Ractor_Imp::run()
{
	  
	  int iHandle=0,b_ret;
	  Socket_Svc_Handler *p_sh=NULL;
	  Socket l_sock;
	  
	  p_sh= newHandler();
	  assert(p_sh != NULL);	 
		while(1)
		{
		 
			 if(!getChecker()->pop_ReadyHandle(iHandle,10))
			 {
			   Pubc::uSleep(10);
			   if(m_bExit == true) 
			   		break;
			   else
			   		continue;
			 }
			 
			 /*
	 		 //cout<<"i_c_i = "<<i_c<<endl;
			 if(iHandle==m_acceptor.getHandle())
			 {
			 	  getChecker()->register_Handle(iHandle);
			 	  if(!m_acceptor.accept(l_sock,1)) 
			 	  	continue;
			 	  iHandle=l_sock.getHandle();
			 	 	p_sh->setHandle(iHandle);
			 	 	b_ret=p_sh->handle_connected();
		  		if(b_ret<0 )
				 	{ 	
				 		p_sh->handle_close();
			 	  	continue;
			 	  };
			 }*/		 	 
		 	
			 p_sh->setHandle(iHandle);
			 b_ret= p_sh->handle_process();

			 if (b_ret == -1)
			 {
			 		getChecker()->remove_Handle(iHandle);
			 	  	p_sh->handle_close(); 
			 }
			 else if(b_ret== -2 )
			 /*需要调用者自己关闭socket 适用于异步处理*/
			 	getChecker()->remove_Handle(iHandle);
			 else
			 	getChecker()->register_Handle(iHandle);
		}
	 		return 0;
};

/*
int Socket_Ractor_Imp::run()
{
	  
	  int iHandle=0,b_ret;
	  Socket_Svc_Handler *p_sh=NULL;
	  Socket l_sock;
	  
	  p_sh= newHandler();
	  assert(p_sh != NULL);	 
		while(1)
		{
		 
			 if(!getChecker()->pop_ReadyHandle(iHandle,10))
			 {
			   Pubc::uSleep(50);
			   if(m_bExit == true) 
			   		break;
			   else
			   		continue;
			 }
			 
			 
	 		 //cout<<"i_c_i = "<<i_c<<endl;
			 if(iHandle==m_acceptor.getHandle())
			 {
			 	  getChecker()->register_Handle(iHandle);
			 	  if(!m_acceptor.accept(l_sock,1)) 
			 	  	continue;
			 	  iHandle=l_sock.getHandle();
			 	 	p_sh->setHandle(iHandle);
			 	 	b_ret=p_sh->handle_connected();
		  		if(b_ret<0 )
				 	{ 	
				 		p_sh->handle_close();
			 	  	continue;
			 	  };
			 }		 	 
		 	
			 p_sh->setHandle(iHandle);
			 b_ret= p_sh->handle_process();
			 if(b_ret!= -1 )
					 	getChecker()->register_Handle(iHandle);
			 else
			 {
			 			getChecker()->remove_Handle(iHandle);
			 	  	p_sh->handle_close();
			 }
		}
	 		return 0;
};*/


inline Socket_Svc_Handler * Socket_Ractor_Imp::getHandler(int id)
{
	// Socket_Svc_Handler *sh=NULL;
	// map<int,Socket_Svc_Handler *>::iterator it = m_mapHandlers.find(id);
	// if(it!=m_mapHandlers.end())
	// {
	// 		sh = (Socket_Svc_Handler *)it->second;
	//		return sh; 
	// }	 
	 return NULL;
};

inline Socket_Svc_Handler * Socket_Ractor_Imp::addHandler(int id)
{
/*
	 Socket_Svc_Handler *sh=getHandler(id);
	 if (sh !=NULL)
	 {
	 		sh->setHandle(id);
			return sh; 
	 }	 
	 sh= newHandler();
	 assert(sh != NULL);
	 m_mapHandlers[id]=sh;
	 sh->setHandle(id);
	 return sh;
*/
  return NULL;
};


//------------------------------Socket_Ractor-------------------------

#ifdef   LINUX
//------------------------------Socket_Epoller-------------------------
Socket_Epoller::Socket_Epoller()
{
	 m_Epoll_Handle=epoll_create(SOCKET_MAXSIZE);
}

Socket_Epoller::~Socket_Epoller()
{
	 if(m_Epoll_Handle >0) close(m_Epoll_Handle);
}

bool Socket_Epoller::register_Handle(int handle)
{
	 ThreadLocker locker(m_mutex_poll);
	 epoll_event  i_event;
	 i_event.events = EPOLLIN | EPOLLET ; 
   i_event.data.fd = handle; 
   

	 if(m_setHandles.find(handle)==m_setHandles.end()) //新连接
	 {
	 	  m_setHandles.insert(handle);
      if (epoll_ctl(m_Epoll_Handle, EPOLL_CTL_ADD, handle, &i_event) < 0) 
      	return false;
	 }
	 else
	 {
	 	  if (epoll_ctl(m_Epoll_Handle, EPOLL_CTL_MOD, handle, &i_event) < 0) 
      	return false;
	 }
	 
	 return true;
}

bool Socket_Epoller::remove_Handle(int handle)
{
	 ThreadLocker locker(m_mutex_poll);
	 epoll_event  i_event;
	 i_event.events = EPOLLIN | EPOLLET; 
   i_event.data.fd = handle; 
   m_setHandles.erase(handle);
   if (epoll_ctl(m_Epoll_Handle, EPOLL_CTL_DEL, handle, &i_event) < 0) 
      	return false;
	 return true;
}


int Socket_Epoller::check_io_once(int p_Millisecond)
{
	 epoll_event a_events[PER_EPOLL_SIZE+1];
	 int i_count=0;
	 ThreadLocker locker(m_mutex_poll);
	 
	 i_count = epoll_wait(m_Epoll_Handle, a_events, PER_EPOLL_SIZE, p_Millisecond);
	 
	 if(i_count > 0)
	 {
	 		for(int i= 0; i < i_count; i++)
	 		{
	 			 if(a_events[i].events & EPOLLIN)
	 			  {
	 			 		m_lstHandles_Ready.push(a_events[i].data.fd);
	 			  }
	 		}
	 }
	 return i_count;
}


bool Socket_Epoller::pop_ReadyHandle(int &handle,int timeout_Millisecond)
{
	 ThreadLocker locker(m_mutex_ready);//线程互斥
	 if(m_lstHandles_Ready.size()==0) 
	 	   check_io_once(timeout_Millisecond);
	 if(m_lstHandles_Ready.size()==0)  return false;
	 	handle=m_lstHandles_Ready.front();
	 	m_lstHandles_Ready.pop();
	 return true;
}

#endif
//------------------------------Socket_Epoller-------------------------


