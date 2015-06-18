/****************************************************************************
** ** Program: This file is part of the Util                 
** Author:  leiqiang  
** Date  :  2006-02-01
**
** RCS: $Id: sss.h,v 1.1.1.1 2006/09/25 01:28:12 sss Exp $
**
****************************************************************************/


#ifndef Util_Log_H
#define Util_Log_H

#include "Pub_c.h"
#include "Singleton.h"
#include "Thread.h"

#include <queue>

//输出屏幕、文件选项。可同时选择
const int LOGOUT_FILE			 = 1;
const int LOGOUT_SCREEN		 = 1 << 1;

//输出级别选项，默认为LOGLEVEL_INFO级别
const int LOGLEVEL_ERROR   = 1;
const int LOGLEVEL_WARN    = 2;
const int LOGLEVEL_INFO    = 3;
const int LOGLEVEL_DEBUG   = 4;
const int LOGLEVEL_TRACE   = 5; 

//提供宏定义，为简单程序代码，特提供编译选项

#if defined(LOG_NOCOMPILE)
	#define  LOG_ERROR(msg)  ;
	#define  LOG_WARN(msg)   ;
	#define  LOG_INFO(msg)   ;
	#define  LOG_DEBUG(msg)  ;
	#define  LOG_TRACE(msg)  ;
	
	#define  LOG_ERROR_FORMAT(fmt,...)  ;
	#define  LOG_WARN_FORMAT(fmt,...)   ;
	#define  LOG_INFO_FORMAT(fmt,...)   ;
	#define  LOG_DEBUG_FORMAT(fmt,...)  ;
	#define  LOG_TRACE_FORMAT(fmt,...)  ;

#else
	#define  LOG_ERROR(msg)  						LogFactory::instance()->log(LOGLEVEL_ERROR,msg) ;
	#define  LOG_WARN(msg)   						LogFactory::instance()->log(LOGLEVEL_WARN,msg) ;
	#define  LOG_INFO(msg)   						LogFactory::instance()->log(LOGLEVEL_INFO,msg) ;
	#define  LOG_DEBUG(msg)  						LogFactory::instance()->log(LOGLEVEL_DEBUG,msg) ;
	#define  LOG_TRACE(msg)  						LogFactory::instance()->log(LOGLEVEL_TRACE,msg) ;
			
	#define  LOG_ERROR_FORMAT(fmt,...)  LogFactory::instance()->log(LOGLEVEL_ERROR,fmt,__VA_ARGS__) ;
	#define  LOG_WARN_FORMAT(fmt,...)   LogFactory::instance()->log(LOGLEVEL_WARN,fmt,__VA_ARGS__) ;
	#define  LOG_INFO_FORMAT(fmt,...)   LogFactory::instance()->log(LOGLEVEL_INFO,fmt,__VA_ARGS__) ;
	#define  LOG_DEBUG_FORMAT(fmt,...)  LogFactory::instance()->log(LOGLEVEL_DEBUG,fmt,__VA_ARGS__) ;
	#define  LOG_TRACE_FORMAT(fmt,...)  LogFactory::instance()->log(LOGLEVEL_TRACE,fmt,__VA_ARGS__) ;
	
#endif //defined(COMPILE_NOLOG)



class Log_File
{
public:
	Log_File();
	~Log_File();
	void 	log (const char * msg,int size = -1); 
	bool	init(const char *LogPath = "./log",
						 const char *logFile = "logfile.log",
						 int				 perfilesize_M  = 5,
						 int				 maxBackupFiles = 10);
private:
	void  	close();
	void 	checkLogFile();
	std::string  getLogFilename(const char* baseName, time_t* now);
	char    m_Path[256];
	char    m_FileName[256]; 
	int		m_curSize;
	int		m_PerFilesize;
	int		m_maxBackupFiles;	
	FILE 	*m_Fp_Log;

	time_t last_roll;
	const static int secondsOfDay = 24*60*60;
	char    baseName[256]; 
	char    linkName[256]; 
};


/*
1、采用独立线程进行IO输出
2、为保证线程安全，需进行线程间互斥，采用多个cache减少互斥的机率
*/

//------------------------------Log_Imp-------------------------
const int  LOG_MAXCACHE =100;
class Log_Imp : public Thread_Base
{
	public:
		Log_Imp();
		~Log_Imp();
		void log (int level,const char *fmt,...);
		bool init(const char *LogPath = "./log",
							const char *logFile = "logfile.log",
							int					logLevle = LOGLEVEL_INFO,
							int					logOutFlag = LOGOUT_FILE|LOGOUT_SCREEN,
							int					perfilesize_M    = 5,
							int					maxBackupFiles   = 10,
							int					cacheperthread_k = 500);
		int	 			stop();
  protected:
  	int					run();
	private:
		void             *getCacheObj(int id);
		int 							m_level;
		int 							m_outFlag;
		void   					 *m_logCacheObj[LOG_MAXCACHE];
		Log_File					m_logFile;
		bool							m_bStop;
		int								m_CachePerThread;
};

typedef Singleton<Log_Imp>  LogFactory;


#endif 
