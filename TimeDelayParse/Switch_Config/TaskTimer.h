#ifndef _UTIL_TIMER_H
#define _UTIL_TIMER_H

#include "Thread.h"
#include "Singleton.h"
#include "DateTime.h"
#include <list>

//定时服务的基类，Timer在定时到达时调用handle_timeout函数，在剩余次数=0时，调用close函数
//基类默认的close函数会delete自己，如果不希望被Timer删除，子类必须覆盖close函数
class TaskTimer_Handler
{
public:
	 virtual int      Timer_timeout()=0;
	 virtual void     Timer_close(){delete this;};
};


class Timer_Handler
{
public:
	 Timer_Handler(TaskTimer_Handler * handler,
	 							 DateTime				    &expire_date,
	 							 int								 count,
	 							 int								 interval_second,
	 							 int								 id):
	 							 m_Handler(handler),
	 							 mExpire_Date(expire_date),
	 							 mLeftCount(count),
	 							 mInterval(interval_second),
	 							 mTimerId(id){};
public:
	 TaskTimer_Handler *m_Handler;
	 DateTime						mExpire_Date;
	 int								mLeftCount;
	 int								mInterval;
	 int								mTimerId;
private:
	 Timer_Handler(){};
};


class  TaskTimer : public Thread_Base
{
public:
	 TaskTimer();
	 ~TaskTimer();
	 //从expire_date时间开始执行，共执行count次，每次之间间隔interval秒
	 //返回值： >=0.正确返回，为本次任务的ID,可用于取消
	 //					<0  失败           
	 int    registerTimer(TaskTimer_Handler *handler,
	 										DateTime				  &expire_date,
	 										int								 count=1,
	 										int								 interval_second=0);
	 void   removeTimer (int timer_id);
	 int    stop() {m_bExit=true;}; 
protected:
	 int   run();
private:
	 int   process_timeout(int &nextsecond);
	 void	 pushTimer(Timer_Handler *th);
	 list<Timer_Handler *> m_lstHanders;
 	 bool  						m_bExit;
 	 int   						m_maxTimerId;
 	 ThreadCondition  m_condTimer;
};

//定义单例Timer
typedef Singleton<TaskTimer>  TaskTimer_Singleton;

#endif

