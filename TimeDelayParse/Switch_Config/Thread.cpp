


#include "Thread.h"
#include <sys/time.h>

ThreadCondition::ThreadCondition()
	{
			m_bSignaled=false;
			pthread_mutex_init(&m_mutex, 0);
		  pthread_cond_init(&m_event, 0);
	};

ThreadCondition::	~ThreadCondition()
	{
			pthread_cond_destroy(&m_event);
		  pthread_mutex_destroy(&m_mutex);
	};

void ThreadCondition::signal()
	{
		 m_bSignaled = true;
		 pthread_cond_signal(&m_event);
	}
	
void ThreadCondition::broadcast()
	{
		 m_bSignaled = true;
		 pthread_cond_broadcast(&m_event);
	}
	
bool ThreadCondition::wait()
	{
		pthread_mutex_lock(&m_mutex);
		while(!m_bSignaled)
					pthread_cond_wait(&m_event, &m_mutex);	
		m_bSignaled=false;
		pthread_mutex_unlock(&m_mutex);
		return true;
	}
	
bool ThreadCondition::wait(int ms)
	{
		pthread_mutex_lock(&m_mutex);
		if (!m_bSignaled)
		{
				struct timespec ts;
				struct timeval cur_tv;
				gettimeofday(&cur_tv, NULL);
	         				
				ts.tv_sec = cur_tv.tv_sec + ms/1000; 
				ts.tv_nsec = cur_tv.tv_usec*1000 + (ms%1000)*1000000;
				if (ts.tv_nsec > 1000000000) { 
					ts.tv_nsec -= 1000000000;
					ts.tv_sec += 1;
				}

				while (!m_bSignaled)
				{ 
					int waitFlag = pthread_cond_timedwait(&m_event, &m_mutex, &ts);
					if (waitFlag != 0)  //time out   
					{
						pthread_mutex_unlock(&m_mutex);
						return false;
					}
				} 
		}
		m_bSignaled=false;
		pthread_mutex_unlock(&m_mutex);
		return true;
	}
	
//--------------------------------------------------------------------

Thread_Base::Thread_Base()
	{
		strcpy(m_sName,"");
	}

Thread_Base::~Thread_Base()
	{
		if (count()>0)
		{
			stop();
			wait();
		}
	};
	
bool Thread_Base::start(int num,pthread_t *thread_ids)
  {
  	ThreadLocker locker(m_mutex);
  	pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr,1048576);
    pthread_t id = 0;
  	for(int i =0;i<num;i++)
  	{
	    if (pthread_create(&id, &attr, &startThread, this) != 0) 
	    		return false;
	    if(NULL!=thread_ids) thread_ids[i]=id;
	  }
		return true;
  }
	
bool Thread_Base::wait()
{
  	 sleep(1);
  	 while(count()>0)  m_cond.wait(5000);
  	 return true;
};

void Thread_Base::uninitialize(pthread_t id)
{
	   ThreadLocker locker(m_mutex);
	   m_ThreadLists.remove(id);
		 m_cond.signal();
		 std::cout<<"thread "<< getName()<< "  id=" <<pthread_self() <<" exit,now num is "<<count() <<std::endl;
}

bool Thread_Base::initialize(pthread_t id)
{
	  ThreadLocker locker(m_mutex);
	  m_ThreadLists.push_back(id);
		pthread_detach(id);
		std::cout<<"thread "<< getName()<< " id=" <<pthread_self() <<" begin runing,now num is "<<count() <<std::endl;
		return true;
}
	
void * Thread_Base::startThread(void* pParam)
	{
		 Thread_Base * pThread = static_cast <Thread_Base*> (pParam);
		 pThread->initialize(pthread_self());
		 pThread->run();
		 pThread->uninitialize(pthread_self());
		 return  NULL;
	}
	

//--------------------------------------------------------------------------------------------

