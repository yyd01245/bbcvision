

#ifndef WINDOWS_SYS
#include <dirent.h>
#endif

#include "Pub_c.h"

namespace Pubc
{

/*!
* \brief  获取去掉扩展名的文件名称
*         
* \param 
*      pFileName： 文件全名
*      pRes     :  去掉扩展名的文件名称(输出)
* \return
*      pRes
*/
char* getHeadName(const char * pFileName,char * pRes)
{
  int id;
  strcpy(pRes,pFileName);
  id=(int)strlen(pRes)-1;
  if(id>0)
  	{
  		while(id>=0&&pRes[id]!='.'&&pRes[id]!='/'&&pRes[id]!='\\') id--;
  		if(pRes[id]=='.')
    	pRes[id]=0;
  	}
  return pRes;
}

char* getHeadName(const char * pFileName)
{
	static char local_str[512];
	return  getHeadName(pFileName,local_str);
}



/*!
* \brief  获取文件扩展名
*         
* \param 
*      pFileName： 文件全名
*      pRes     :  扩展名(输出)
* \return
*      1:  SUCCESS
*      0:  FAILURE
*/
char * getExtName(const char * pFileName,char * pRes)
{
	int id;
  id=(int)strlen(pFileName)-1;
  if(id>=0)
  {
    while(id>=0&&pFileName[id]!='.'&&pFileName[id]!='/'&&pFileName[id]!='\\') id--;
  	if(pFileName[id]=='.')
  	{
    	strcpy(pRes,pFileName+id+1);
  	};
  }
  return pRes;
}

char * getExtName(const char * pFileName)
{
	static char local_str[512];
  return  getExtName(pFileName,local_str);
}

/*!
* \brief  得到不包含路径的文件名
*         
* \param 
*      pFullName： 全路经文件名
*      pRes     :  不包含路径的文件名(输出)
* \return
*      1:  SUCCESS
*      0:  FAILURE
*/
char * getShortName(const char * pFullName,char * pRes)
{
  int id;
  id=(int)strlen(pFullName)-1;
  while(id>=0&&pFullName[id]!='/'&&pFullName[id]!='\\') id--;
  if(pFullName[id]=='/' || pFullName[id]=='\\')
  	 strcpy(pRes,pFullName+id+1);
  else
  	 strcpy(pRes,pFullName);
  
  return pRes;
}

char * getShortName(const char * pFullName)
{
	static char local_str[512];
  return  getShortName(pFullName,local_str);
}




/*!
* \brief  判断文件是否存在
*         
* \param 
*      pFileName： 文件全名
*      
* \return
*      1:  存在
*      0:  不存在
*/
bool isFileExist(const char * pFileName)
{
FILE  *fp;

  if((fp=fopen(pFileName,"rb"))!=NULL)
  {
    fclose(fp);
    return true;
  }
  else
    return false;
}



/*!
* \brief  根据句柄取文件长度
*         
* \param 
*      handle   :  文件句柄
*      
* \return
*      >0:  文件长度
*      -1:  失败
*/
int getFileSize(int handle)
{
struct stat statbuf;

  if(fstat(handle,&statbuf)!=0)
    return -1;
  else
    return statbuf.st_size;
}


/*!
* \brief  根据句柄取文件长度
*         
* \param 
*      pFileName   :  文件名称
*      
* \return
*      >0:  文件长度
*      -1:  失败
*/
int getFileSize(const char * pFileName)
{
struct stat statbuf;

  if(stat(pFileName,&statbuf)!=0)
    return -1;
  else
    return statbuf.st_size;
}


/*!
* \brief  判断路径是否存在
*         
* \param 
*      path    : 路径名
*      
* \return
*      1:  存在
*      0:  不存在
*/
#ifdef WINDOWS_SYS
#include <io.h>
#endif

bool isPathExist(const char * pPath)
{
#ifdef WINDOWS_SYS
  if((access( pPath, 0 )) != -1 )
      return true;
    return false;
#else
DIR * dirp;

  dirp=opendir(pPath);
  if(dirp==NULL)
  {
    return false;
  };
  closedir(dirp);
  return true;
#endif
}



// create a directory
/*!
* \brief  创建目录
*         
* \param 
*      path    : 目录名,支持多级目录
*      
* \return
*      1:  成功
*      0:  失败
*/
/*bool createDir(const char path[]) 
{
    char strRDir[256];
    mode_t mMode = 0777;

    if (isPathExist(path))
    {
        return true;
    }
    else
    {
        if(mkdir(path,mMode) == 0)
        {
            return true;
        }
        else
        {
            memset(strRDir,0,sizeof(strRDir));
            getShortName(path,strRDir);
            if(strcmp(strRDir,"/") == 0)
                return false;
            if(createDir(strRDir))
                return createDir(path);
            else
                return false;
        }
    }
}
*/
//-------------------------------------------------------------------------------------------

/*
函数功能：将字符转换为大写
传入参数：
  src 待转换字符串
*/
char * upper(char * src)
{
  if(src==NULL) return src;
  char  *pos=src;
  while(*pos!=0)
  {
    if(*pos>='a'&&*pos<='z')
      *pos=(char)(*pos-'a'+'A');
    pos++;
  }
  return src;
}

/*
函数功能：将字符转换为小写
传入参数：
  src 待转换字符串
*/
char * lower(char * src)
{
  if(src==NULL) return src;
  char  *pos=src;
  while(*pos!=0)
  {
    if(*pos>='A'&&*pos<='Z')
      *pos=(char)(*pos-'A'+'a');
    pos++;
  }
  return src;
}

char * firstUpper(char * p_str)
{
		lower(p_str);
    int len = strlen( p_str);
    bool Pre_is_Letter = false;
    for (int i=0; i < len ;i++ )
    {
        if (p_str[i]>='a' && p_str[i]<='z' )
        	{
        		if (Pre_is_Letter == false)
        		{
        			 p_str[i]=p_str[i]+'A' - 'a';
        			 Pre_is_Letter == true;
        		}
        	}
        else
        	{
        		 Pre_is_Letter == false;
        	}
    }    
    return p_str;
}

char * trimLeft(char * psrc)
{
	char * src,*tag;
  if(psrc==NULL) return psrc;
  tag=src=psrc;
  while(*src==' '||*src==0x09||*src==0x0a||*src==0x0d)  src++;
  if(tag!=src)
  {
  	while(*src!=0) *tag++=*src++;
  	*tag=0;
  }	
  return psrc;
}

char * trimRight(char * src)
{
  char *pos;

  if(src==NULL) return src;
  pos=src+strlen(src) -1 ;
  while(pos>=src)
  {
    if(*pos==' '||*pos==0x09||*pos==0x0a||*pos==0x0d)
    	*pos=0;
    else
    		break;
    pos--;
  };
  return src;
}

char * trim (char * src)
{
  trimLeft(src);
  trimRight(src);
  return src;
}

/*!
* \brief  模拟ORACLE的like
*           当前支持%多字符通配符,'_'单字符通配符，
*           '%'表示成'\%','\'表示成'\\','_'表示成'\_'
*         
* \param 
*      src_str  :源字符传
*      pattern  :要匹配的格式
*      
* \return
*      1 :匹配
*      0 :不匹配
*     
*/
inline bool  likeHead(const char *src_str,const char*pattern)
{
    char *str=(char *)src_str;
    char *pa_str=(char *)pattern;
    char ch=0;
    while(*pa_str!='\0'&&*str!='\0')
    {
        ch=*pa_str++;
        switch(ch)
        {
            case '_':
                 str++;
                 break;
            case '\\':
               ch=*pa_str++;
            default :
               if(*str++!=ch)
             return false;
         break ;
        }
    }
    //参数字段结束
    if(*pa_str!='\0') return false;
    return true;
    
}

bool  like(const char *src_str,const char*pattern)
{
    char pa_buf[1024];
    char *str=(char *)src_str;;
    char *pa_str=(char *)pattern;;
    char  ch=0;
    unsigned int   pa_len=0;

    // 1234%
    while(*pa_str!='\0'&&*str!='\0')
    {
        ch=*pa_str++;
        if(ch=='%') break;
        if(ch=='_') 
        {
        *str++;
        continue;
        }
        if(ch=='\\') ch=*pa_str++;
        if(*str++!=ch) return false;
    }
    
    //%......    
    while(*pa_str!='\0'&&*str!='\0')
    {
        pa_len=0;
        while(*pa_str!='\0'&&*pa_str!='%')
            {
                if(*pa_str=='\\') pa_str++;
                pa_buf[pa_len++]=*pa_str++;
            }   
        pa_buf[pa_len]=0;
    
        if(strlen(str) < pa_len ) return false;
        //%12111
        if(*pa_str=='\0')
        {
            str=str+strlen(str)- pa_len;
            return likeHead(str,pa_buf);
        }   
        //%121313%
        while(*str!='\0'&&pa_len >0)
        {
            if(strlen(str)< pa_len) return false;
            if(likeHead(str,pa_buf)) break;
             str++;
        }   
        //查找到121313
        str+=pa_len;
        pa_str++;
    }
        
    return true;
}


/*!
* \brief  将字符串中的一段用另外一段替换,注意strbuf必须有足够的空间来存储替换后的串,否则可能内存溢出
*         
* \param 
*      strbuf : 字符串(输入/输出)
*      src_str : 目标字符子串
*      desc_str     : 替换后的子串
* \return
*      替换后的字符串
*     
*/
char * replace(char *strbuf,const char *src_str,const char *desc_str)
{
    
    char *pos,*pos1;
    
    if (strbuf==NULL||src_str==NULL||desc_str==NULL) return strbuf;
    
    if(strlen(src_str)==0) return strbuf;
    char *org=new char[strlen(strbuf)+1];
       
    strcpy(org,strbuf);
    pos=org;
    strbuf[0]=0;
    while(1)
    {
        pos1=strstr(pos,src_str);
        if(pos1==NULL)
        break;
        *pos1='\0';
        strcat(strbuf,pos);
        strcat(strbuf,desc_str);
        pos=pos1+strlen(src_str);       
    }
    strcat(strbuf,pos);
    delete org;
    return strbuf;
}


//--------------------------------------------------------------------------------------------
void uSleep(int num)
{
  struct timeval tm;

  tm.tv_sec = 0;
  if(num<=0) num=5;
  tm.tv_usec = num*1000;
  select(0, NULL, NULL, NULL, &tm);
};

//--------------------------------------------------------------------------------------------
void * loadDllFunc(const char *pLib,const char *pModule)
{
/*	 void * m_pHandle=NULL;
	 if(!pLib || !pModule) return NULL;
	 m_pHandle=dlopen(pLib,RTLD_LAZY);
	 if(NULL!=m_pHandle)
	   return (void *)dlsym(m_pHandle,pModule);
*/	 return NULL;	 
}
//--------------------------------------------------------------------------------------------
char * moneyToCapital(int amount,char * res_str)
{
char  upperdigital[][3]={"零","壹","贰","叁","肆","伍","陆","柒","捌","玖"};
char  units[][3]={"分","角","元","拾","佰","仟","万","拾","佰","仟","亿","拾","佰","仟"};
char  samount[32],upperres_str[64];
int   flag=0, pos;
static char loc_res_str[200];

  if(res_str==NULL) res_str=loc_res_str;

  memset(upperres_str,'\0',64);
  sprintf(samount,"%d.%02d",amount/100,amount%100);

  for(int digitalid=(int)strlen(samount)-1;digitalid>=0;digitalid--)
  {
    if(samount[(int)strlen(samount)- digitalid - 1]!='.')
    {
      pos=digitalid>2?digitalid-1:digitalid;
      if(samount[(int)strlen(samount)- digitalid - 1]!='0')
      {
        strcat(upperres_str,upperdigital[int(samount[(int)strlen(samount) - digitalid - 1]-'0')]);
        strcat(upperres_str,units[pos]);
        flag=1;
      }
      else
      {
        switch(pos)
        {
          case 10:/*亿的特殊处理*/
            if(flag)
            {
              if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"零",2)==0)/*去掉亿前的零*/
                upperres_str[(int)strlen(upperres_str)-2]='\0';
              strcat(upperres_str,"亿零");
            }
            break;
          case 6:/*万的特殊处理*/
            if(flag)
            {
              if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"零",2)==0)/*去掉万前的零*/
                upperres_str[(int)strlen(upperres_str)-2]='\0';
              /*如果前面是亿，则加零*/
              if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"亿",2)==0)/*如果没有万位则用零*/
                strcat(upperres_str,"零");
              else
                strcat(upperres_str,"万零");
            }
            break;
          case 2:/*元的特殊处理*/
            if(flag)
            {
              if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"零",2)==0)/*去掉元前的零*/
                upperres_str[(int)strlen(upperres_str)-2]='\0';
                strcat(upperres_str,"元零");
            }
            else
                strcat(upperres_str,"零元");
            break;
          default:
            if(flag)
              if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"零",2)!=0)
                strcat(upperres_str,"零");
          break;
        }
      }
    }
  }
  /*事後处理*/
  if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"零",2)==0)
    upperres_str[(int)strlen(upperres_str)-2]='\0';
  if(memcmp(upperres_str+(int)strlen(upperres_str)-2,"分",2)!=0)
    strcat(upperres_str,"整");
 
  strcpy(res_str,upperres_str);
  return res_str;
};

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
}
