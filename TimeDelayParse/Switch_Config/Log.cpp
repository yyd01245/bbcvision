
/**************************************************************************
 * \file    Log.cpp
 * \brief   
 *
 * Copyright (c) 2006 OnewaveInc.
 * RCS: $Id: rate_sock.cpp,v 1.9 2008/08/06 10:39:17 chenjx Exp $
 *
 * History
 *  2006/10/11 leiq     created
**************************************************************************/


#include "Log.h"
#include "DateTime.h"
#include <stdarg.h>

#include <sys/time.h> 
#include <time.h>

const int LOG_PERMAXSIZE = 10*1024;//每次输出不大于10k,用于防止内存溢出

//------------------------------LogThreadCache-------------------------
/*
   线程临时日志缓存，内部采用循环内存模式，即如果来不及输出，则会覆盖原来的日志
   内存缓存初始化默认为500K。
   注意:约定每次写入不得超过LOG_PERMAXSIZE字节(10k),作用是防止内存溢出
*/
class LogThreadCache
{
public:
	static LogThreadCache * newObj(int cachesize,int maxpersize=LOG_PERMAXSIZE)
	{
		return new LogThreadCache(cachesize,maxpersize);
	};
	~LogThreadCache() {	if(mBuf!=NULL) delete mBuf;mBuf =NULL;};
	char *writePtr()	{ return mBuf+m_size; };//返回可写的地址
	char *basePtr() const { return mBuf;}; //返回缓存首地址
	int		getSize(){return m_size;};
	void  addSize(int len)
	{
		 m_size+=len;
		 if(m_size > m_CacheSize ) //进入下一循环
		 	 m_size=0;
		 mBuf[m_size]='\0';
	};
	void	reset(){m_size=0;mBuf[0]='\0';};
	void  lock(){m_mutex.lock();};
	void  unlock(){m_mutex.unlock();};
private:
	LogThreadCache();
	LogThreadCache(int cachesize,int maxpersize)
	{
		  m_CacheSize=cachesize > LOG_PERMAXSIZE ? cachesize:LOG_PERMAXSIZE;
		  m_MaxPerSize=maxpersize > LOG_PERMAXSIZE ? maxpersize:LOG_PERMAXSIZE;
		  mBuf= new char[m_CacheSize+m_MaxPerSize];
		  m_size=0;
	}	
	char 				  *mBuf;
	int						 m_size;
	int						 m_CacheSize;
	int						 m_MaxPerSize;
	ThreadMutex  	 m_mutex;
};

//------------------------------LogThreadCache-------------------------


//------------------------------Log_File-------------------------
Log_File::Log_File()
{
	m_Fp_Log=NULL;
	m_curSize=0;
	strcpy(m_Path,"./logs");
	sprintf(m_FileName,"%s/logfile.log",m_Path);
	m_PerFilesize=1024*1024;
	m_maxBackupFiles=10;
}

Log_File::~Log_File()
{
	close();
}

void Log_File::close()
{
	if(m_Fp_Log!=NULL)
		fclose(m_Fp_Log);
	m_Fp_Log=NULL;
	m_curSize=0;
}

std::string Log_File::getLogFilename(const char* baseName, time_t* now)
{
	string filename;
	filename.reserve(strlen(baseName) + 32);
	filename = baseName;

	char timebuf[32];
	struct tm tm;
	*now = time(NULL);
	localtime_r(now, &tm);
	strftime(timebuf, sizeof timebuf, ".%Y-%m-%d.%H%M%S", &tm);
//	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
	filename += timebuf;
//	filename += "log";
	
	return filename;
}

bool Log_File::init(const char *LogPath,
						 				const char *logFile,
						 				int				  perfilesize_M,
						 				int				  maxBackupFiles)
{
	time_t now_time;
	
	strncpy(m_Path,LogPath,512);
	
	//cout<<" perfilesize_M = "<<perfilesize_M<<endl;
	m_PerFilesize = perfilesize_M <1000 ? perfilesize_M:1000;
	m_PerFilesize = m_PerFilesize * 1024 *1024;
	
	//cout<<" m_PerFilesize = "<<m_PerFilesize<<endl;

	m_maxBackupFiles =maxBackupFiles <100?maxBackupFiles:100;
	
	if(strlen(m_Path)>0)
	{
		while(m_Path[strlen(m_Path) - 1]=='/')  
			m_Path[strlen(m_Path) - 1]='\0';
	}
	
	for(int i=0;i<2;i++)
	{
		if(!Pubc::isPathExist(m_Path))
		{
				char buf[1024];
				sprintf(buf,"mkdir -p %s",m_Path);
				system(buf);
		}
		if(!Pubc::isPathExist(m_Path))
			strcpy(m_Path,"./logs");
	}

	sprintf(baseName, "%s", logFile);
	sprintf(linkName, "%s/%s", m_Path, baseName);
	getLogFilename(baseName, &now_time);
	last_roll = (now_time + 8*3600)/ secondsOfDay * secondsOfDay;
	
	return Pubc::isPathExist(m_Path);	
}


inline void Log_File::checkLogFile()
{
	//cout<<"m_curSize= "<<m_curSize<<"  m_PerFilesize = " <<m_PerFilesize<<endl;
	time_t now, new_roll;
	now = time(NULL);
	new_roll = (now + 8*3600) / secondsOfDay * secondsOfDay;
	if(m_Fp_Log!=NULL&&m_curSize < m_PerFilesize && (new_roll == last_roll))
		 return;
	
	close();

	string logname =  getLogFilename(baseName, &now);
	sprintf(m_FileName, "%s/%s", m_Path, logname.c_str());
	last_roll = new_roll;
	
	//创建文件
	m_Fp_Log=fopen(m_FileName,"wt");

	unlink(linkName);
	symlink(logname.c_str(), linkName);
}

void Log_File::log (const char * msg,int size)
{
	int length=0;
	if(size<=0)
	     length=strlen(msg);
	else
			 length=size;	
	checkLogFile();
	if(m_Fp_Log!=NULL)
	{
		fwrite(msg,length,1,m_Fp_Log);
		fflush(m_Fp_Log);
	  m_curSize+=length;
	}
	
	checkLogFile();		 
}

//------------------------------Log_File-------------------------
//------------------------------Log_Imp-------------------------
Log_Imp::Log_Imp()
{
	 m_bStop=false;
   m_level=LOGLEVEL_INFO;
	 m_outFlag = LOGOUT_FILE;
	 m_CachePerThread = 500 *1024;
	 setName("Log_Imp");
	 
	for(int i=0;i<100;i++)
	  m_logCacheObj[i]=NULL;
}

Log_Imp::~Log_Imp()
{
	stop();
	wait();
	for(int i=0;i<100;i++)
	 if(m_logCacheObj[i]!=NULL)
	 	  delete (LogThreadCache *)m_logCacheObj[i]; 
}

bool Log_Imp::init(const char *LogPath,
										const char *logFile ,
										int					logLevle,
										int					logOutFlag,
										int					perfilesize_M,
										int					maxBackupFiles,
										int					cacheperthread_k)
{
	 m_level=logLevle;
	 m_outFlag = logOutFlag;
	 m_CachePerThread = cacheperthread_k *1024;
	 return m_logFile.init(LogPath,logFile,perfilesize_M,maxBackupFiles);
};

inline void *Log_Imp::getCacheObj(int id)
{
	 if(id<0 || id > LOG_MAXCACHE - 1) return NULL;
	 if(m_logCacheObj[id]!=NULL)
	 		return m_logCacheObj[id];
	 
	 ThreadLocker locker(m_mutex);
	 if(m_logCacheObj[id]==NULL)
	 	  m_logCacheObj[id]=(void *)LogThreadCache::newObj(m_CachePerThread);
	 return m_logCacheObj[id];	 	  
};

void Log_Imp::log (int level,const char *fmt,...)
{
	 if (level > m_level || m_bStop==true) return;
	 
	 int id=pthread_self()%LOG_MAXCACHE;
	 //int id=0;
	 LogThreadCache *cache=(LogThreadCache *)getCacheObj(id);
	 if(cache==NULL) return;
	 
	 va_list ap;
	 cache->lock();
	 DateTime d;
	 char *buf=cache->writePtr();
	struct timeval tv;
	gettimeofday(&tv, NULL); 
	struct tm *ptm = localtime(&tv.tv_sec);
	sprintf(buf,"[%4d-%02d-%02d %02d:%02d:%02d:%07.3f] ",(1900+ptm->tm_year),(1+ptm->tm_mon),
		ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tv.tv_usec/1000.0);
	 //sprintf(buf,"%s : ",d.to_string("yyyy-mm-dd hh:nn:ss").c_str());
	 cache->addSize(strlen(buf));
	 
	 va_start(ap, fmt);
	 buf=cache->writePtr();
	 
	 vsprintf(buf , fmt, ap);
	 cache->addSize(strlen(buf));
	 va_end(ap);
	 cache->unlock();
	 
	 //m_queueLog.push(id);	 	  
};


int Log_Imp::stop()
{
	 m_bStop=true;	 	  
};

int Log_Imp::run()
{
		int i_count=0;
		LogThreadCache *cache=NULL;
 
		while(m_bStop==false)
	  {
	  	i_count=0;
	  	for(int i =0;i<LOG_MAXCACHE;i++)
	  	{
	  		 cache=(LogThreadCache *)getCacheObj(i);
	  		 if(cache==NULL) continue;
	  		 if(cache->getSize()<=0) continue;
	  		 i_count++;
	  		 cache->lock();
	  		 if(BIT_ENABLED(m_outFlag,LOGOUT_SCREEN))
	  		 	 cout<<cache->basePtr()<<endl;
	  		 if(BIT_ENABLED(m_outFlag,LOGOUT_FILE))
				 	 m_logFile.log(cache->basePtr(),cache->getSize());
	  		 cache->reset();
	  		 cache->unlock();
	  	}
	  	if(i_count <10)
	  		 Pubc::uSleep(10); 	
	  	if(i_count == 0)
	  		 Pubc::uSleep(100);
	  }
	  return 0;	  			 
}

									



