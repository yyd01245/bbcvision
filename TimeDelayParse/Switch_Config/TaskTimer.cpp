
/**************************************************************************
 * \file    TaskTimer.cpp
 * \brief   
 *
 * Copyright (c) 2006 OnewaveInc.
 * RCS: $Id: ,v 1.9 2008/08/06 10:39:17 chenjx Exp $
 *
 * History
 *  2006/10/11 leiq     created
**************************************************************************/


#include "TaskTimer.h"


//------------------------------Timer-------------------------

TaskTimer::TaskTimer()
{
	 m_bExit=false;
	 m_maxTimerId=0;
}

TaskTimer::~TaskTimer()
{
	while(!m_lstHanders.empty())
	 {
	 	 Timer_Handler * handler=m_lstHanders.front();
	 	 handler->m_Handler->Timer_close();
 	  m_lstHanders.pop_front();	 	 
	 }
	
}

int TaskTimer::run()
{
	 int i_count=0,i_nextsec=0;
	 while(m_bExit==false)
	 {
	 	  i_count=process_timeout(i_nextsec);
	 	  if(i_nextsec<=0) i_nextsec=1;
	 	  m_condTimer.wait(i_nextsec *1000);
	 }
	 return 0;
}

int TaskTimer::registerTimer(TaskTimer_Handler *handler,
	 										DateTime				  &expire_date,
	 										int								 count,
	 										int								 interval_second)
{
	 
	 m_maxTimerId++;
	 m_maxTimerId%=2000000000;
	 Timer_Handler *th= new Timer_Handler(
	 												handler,
	 												expire_date,
	 												count,
	 												interval_second,
	 												m_maxTimerId);
	 if (th==NULL) return -1;
	 ThreadLocker locker(m_mutex);//线程互斥
	 pushTimer(th);
	 return m_maxTimerId;
}

void TaskTimer::removeTimer (int timer_id)
{
	 list<Timer_Handler *>::iterator it;
	 ThreadLocker locker(m_mutex);
	 for(it= m_lstHanders.begin();
	     it != m_lstHanders.end();
	     it ++)
	 {
	 	  if ((*it)->mTimerId == timer_id)
	 	  {
	 	  	 (*it)->m_Handler->Timer_close();
	 	  	 m_lstHanders.erase(it);
	 	  	 break;
	 	  }
	 }
}

int TaskTimer::process_timeout (int &nextsecond)
{
	 int i_count=0,ret;
	 list<Timer_Handler *> tmp_lst;
	 nextsecond=0;
	 while(!m_lstHanders.empty())
	 {
	 	 ThreadLocker locker(m_mutex);
	 	 if(m_lstHanders.empty()) break;
	 	 
	 	 Timer_Handler * handler=m_lstHanders.front();
	 	 nextsecond=handler->mExpire_Date.getTimet() - DateTime::Now();
	 	 if(nextsecond>0)  break;
	 	 ret=handler->m_Handler->Timer_timeout();
	 	 handler->mLeftCount--;
	 	 i_count++;
	 	 if(handler->mLeftCount <=0 || ret <0)
		 	{
		 		handler->m_Handler->Timer_close();
		 	}
		 else
		 	{
		 		handler->mExpire_Date.addSec(handler->mInterval);
		 		tmp_lst.push_back(handler);
			}	 	 
	 	  m_lstHanders.pop_front();	 	 
	 }
	 while(!tmp_lst.empty())
	 {
	 	  pushTimer(tmp_lst.front());
	 	  tmp_lst.pop_front();
	 };
	 return 	i_count;
}

//此函数确保head是最早应该被触发的对象
void TaskTimer::pushTimer (Timer_Handler * th)
{
	if(m_lstHanders.empty())
	{
		m_lstHanders.push_front(th);
		m_condTimer.signal();
		return ;
	}
	list<Timer_Handler *>::iterator it =m_lstHanders.begin();
	for(;it!=m_lstHanders.end();it++)
	{
		 if((*it)->mExpire_Date >= th->mExpire_Date) break;
	}
	m_lstHanders.insert(it, th);
	m_condTimer.signal();
	return;
}


//------------------------------Timer-------------------------

