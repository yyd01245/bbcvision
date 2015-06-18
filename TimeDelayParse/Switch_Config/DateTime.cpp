/* -*- C++ -*- */

#include "DateTime.h"
#include <stdio.h>

int DateTime_get_int(const char * p_time,const char *fmt,const char *para)
{
	const char *pos;
	char buf[20];
	
	pos=strstr(fmt,para);
	if(pos!=NULL)
	{
     memcpy(buf,p_time + (pos - fmt),strlen(para));
     buf[strlen(para)]='\0';
     return atoi(buf);
  }
  return -1;
}

int DateTime::getMonthDays(int year,int month)
{
    int days;
    
    switch(month)
    {
        case 1 :
        case 3 :
        case 5 :
        case 7 :
        case 8 :
        case 10:
        case 12:
             days=31;
             break;
        case 2:
             if((year%4==0&&year%100!=0)||year%400==0)
              days=29;
             else
              days= 28;
             break; 
        default:
             days= 30;
             break;
        }
  return days;  
}

bool   DateTime::init(int year,int month,int day,int hour,int minute,int second)
{
		struct tm m;
		m.tm_year=year;
		m.tm_mon=month;
		m.tm_mday=day;
		m.tm_hour=hour;
		m.tm_min=minute;
		m.tm_sec=second;
 		return init(mktime(&m));
}

bool   DateTime::init(const char * p_time,const char *fmt)
{
	
		struct tm m;
		m.tm_year=m.tm_mon=m.tm_mday=m.tm_hour=m.tm_min=m.tm_sec=-1;
		
		char s_fmt[100],*pos,buf[20];
		if(NULL!= fmt)
			strncpy(s_fmt,fmt,50);
		else
			strcpy(s_fmt,"yyyymmddhhnnss");
		Pubc::lower(s_fmt);
		
		if(NULL==p_time || strlen(p_time)!=strlen(s_fmt)) return false;
			
		//年
		m.tm_year=DateTime_get_int(p_time,s_fmt,"yyyy");
	  if (m.tm_year < 0)  m.tm_year=DateTime_get_int(p_time,s_fmt,"yy");
	  if(m.tm_year > 1900) m.tm_year-=1900;
	  if(m.tm_year > 50 && m.tm_year <100 ) m.tm_year+=1900;
	  if(m.tm_year < 0  || m.tm_year >5000 ) return false;
	 
	 	//月   	
	 	m.tm_mon=DateTime_get_int(p_time,s_fmt,"mm");
	 	m.tm_mon--;
    if(m.tm_mon < 0  || m.tm_mon >11 ) return false;
 		//日 
 		m.tm_mday=DateTime_get_int(p_time,s_fmt,"dd");
    if(m.tm_mday < 1  || m.tm_mday >31 ||  m.tm_mday > getMonthDays(m.tm_year,m.tm_mon+1)) return false;
    //时
 		m.tm_hour=DateTime_get_int(p_time,s_fmt,"hh");
    if(m.tm_hour < 0  || m.tm_hour >24) return false;
    //分
 		m.tm_min=DateTime_get_int(p_time,s_fmt,"nn");
    if(m.tm_min < 0  || m.tm_min >59) return false;
    //秒
 		m.tm_sec=DateTime_get_int(p_time,s_fmt,"ss");
    if(m.tm_sec < 0  || m.tm_sec >59) return false;
 
 		return init(mktime(&m));
}


string  DateTime::to_string(const char *fmt) const
{
	string buf;
	get_string(buf,fmt);
  return buf;
}		

bool    DateTime::get_string(string &buf,const char *fmt) const
{
	 char c_buf[20];
	 sprintf(c_buf,"%04d%02d%02d%02d%02d%02d",getYear(),getMonth(),getDay(),getHour(),getMinute(),getSecond());
	 
	 	if(fmt==NULL)
	 	{
	 		buf=c_buf;
	 		return true;
	 	}
	
 		char *pos,c_tmp[100];
 		strcpy(c_tmp,fmt);
 		pos=strstr(c_tmp,"yyyy");
 		if(NULL!=pos)  memcpy(pos,c_buf,4);
 		pos=strstr(c_tmp,"yy");
 		if(NULL!=pos)  memcpy(pos,c_buf+2,2);
 		pos=strstr(c_tmp,"mm");
 		if(NULL!=pos)  memcpy(pos,c_buf+4,2);
 		pos=strstr(c_tmp,"dd");
 		if(NULL!=pos)  memcpy(pos,c_buf+6,2);
 		pos=strstr(c_tmp,"hh");
 		if(NULL!=pos)  memcpy(pos,c_buf+8,2);
 		pos=strstr(c_tmp,"nn");
 		if(NULL!=pos)  memcpy(pos,c_buf+10,2);
 		pos=strstr(c_tmp,"ss");
 		if(NULL!=pos)  memcpy(pos,c_buf+12,2);	 				 				 			

	  buf=c_tmp;
	 
	  return true;
}


int DateTime::getMonthbyEng(const char * engmon)
{
  char line[500];

  strcpy(line,engmon);
  Pubc::lower(line);
  
  if(strstr(line,"jan")!=NULL) return 1;
  if(strstr(line,"feb")!=NULL) return 2;
  if(strstr(line,"mar")!=NULL) return 3;
  if(strstr(line,"apr")!=NULL) return 4;
  if(strstr(line,"may")!=NULL) return 5;
  if(strstr(line,"jun")!=NULL) return 6;
  if(strstr(line,"jul")!=NULL) return 7;
  if(strstr(line,"aug")!=NULL) return 8;
  if(strstr(line,"sep")!=NULL) return 9;
  if(strstr(line,"oct")!=NULL) return 10;
  if(strstr(line,"nov")!=NULL) return 11;
  if(strstr(line,"dec")!=NULL) return 12;
  return 1;
};
