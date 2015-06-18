/* -*- C++ -*- */

/****************************************************************************
** ** Program: This file is part of the Util                 
** Author:  leiqiang  
** Date  :  2006-02-01
**
** RCS: $Id: sss.h,v 1.1.1.1 2006/09/25 01:28:12 sss Exp $
**
****************************************************************************/

#ifndef UTIL_DATETIME_H
#define UTIL_DATETIME_H

using namespace std;

#include <string>
#include <time.h>
#include <iostream>
#include "Pub_c.h"

class DateTime 
{
public:
	DateTime ()
		{init(time(NULL));};
	DateTime (time_t sec)
		{init(sec);};
	DateTime (tm &t)
		{init(mktime(&t));};
	
	static time_t Now() { return time(NULL);}
	static string Now(const char *fmt)
	{
		 DateTime d(DateTime::Now());
		 return d.to_string(fmt);
	};
	
	bool   init(time_t sec)
		{ 
		 if (sec < 0) return false; 
		 m_sec=sec;
		 struct tm *local;
		 local=localtime(&m_sec);
		 memcpy(&m_t,local,sizeof m_t);
		 return true;		 
		};
	
	bool   init(int year,int month,int day,int hour,int minute,int second);
		
	/*
	  年：yyyy/yy  月：mm   日：dd  时：hh  分: nn  秒：ss
	  如果fmt为空，则为yyyymmddhhnnss  
	*/
	bool     init(const char * p_time,const char *fmt=NULL);
	//注：虽然to_string这种用法很简单，但多了clone string,性能有所下降,建议使用get_string
	string   to_string(const char *fmt=NULL) const;
	bool     get_string(string &buf,const char *fmt=NULL) const;

	bool   addSec(time_t sec){return init(getTimet()+sec);};
	
	time_t getTimet() const  {return m_sec;};	
	int getYear() const    {return m_t.tm_year+1900;};
	int getMonth() const   {return m_t.tm_mon+1;};
	int getDay() const     {return m_t.tm_mday;};
	int getHour() const    {return m_t.tm_hour;};
	int getMinute() const  {return m_t.tm_min;};
	int getSecond() const  {return m_t.tm_sec;};
	int getWeekDay() const {return m_t.tm_wday==0?7:m_t.tm_wday;};
	
	DateTime& operator = (const DateTime& t1)
	{
		if( this != &t1 )
		{
			 init(t1.getTimet());
		}
		return *this;
	};
	
	bool operator<(const DateTime& t1)  const {return m_sec < t1.getTimet();};
	bool operator>(const DateTime& t1)  const {return m_sec > t1.getTimet();};
	bool operator<=(const DateTime& t1) const {return m_sec <= t1.getTimet();};
	bool operator>=(const DateTime& t1) const {return m_sec >= t1.getTimet();};
	bool operator==(const DateTime& t1) const {return m_sec == t1.getTimet();};
	bool operator!=(const DateTime& t1) const {return m_sec != t1.getTimet();};
	
	friend ostream& operator<<( ostream& d, const DateTime &dt )
	{

		d  << dt.to_string();
		return d;
	}
	

	void   clear()
	{
		 m_sec=0;
		 memset(&m_t,0,sizeof m_t);
	}
	
	static int   getMonthDays(int nYear, int nMonth);
	static int   getMonthbyEng(const char * engmon);
	static int   getYearDays(int year)
  {
  		if((year%400==0)||((year%4==0)&&(year%100!=0))) /*润年*/
    		return 366;
  		else
    	return 365;
	}
		
private:
	time_t    m_sec;
	struct tm m_t;
};

#endif

