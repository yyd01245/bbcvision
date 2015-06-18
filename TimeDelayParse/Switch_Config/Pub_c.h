/****************************************************************************
** ** Program: This file is part of the Util                 
** Author:  leiqiang  
** Date  :  2006-02-01
**
** RCS: $Id: sss.h,v 1.1.1.1 2006/09/25 01:28:12 sss Exp $
**
****************************************************************************/

#ifndef pub_c_h
#define pub_c_h

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

#include "Global_Macros.h"

namespace Pubc
{

  //文件操作类函数,注意部分函数涉及static变量，是线程不安全的
	extern char* getHeadName(const char * pFileName,char * pRes);  //获取去处扩展名后的文件名称
	extern char* getHeadName(const char * pFileName);  						 //同上，线程不安全
	extern char* getExtName(const char * pFileName,char * pRes);   //获取文件扩展名
  extern char* getExtName(const char * pFileName);               //同上，线程不安全
	extern char* getShortName(const char * pFullName,char * pRes); //得到不包含路径的文件名
	extern char* getShortName(const char * pFullName);             //同上，线程不安全
	extern bool  isFileExist(const char * pFileName);                   //判断文件是否存在	
	extern int   getFileSize(int fd);                                   //根据句柄取文件长度
	extern int   getFileSize(const char * pFileName);                   //根据文件名称取文件长度
	extern bool  isPathExist(const char * pPath);                       //判断路径是否存在




  //字符操作函数
  extern char * upper(char * pStr);     		 	//转换为大写字符串,破坏原有数据
	extern char * lower(char *pStr);      		 	//转换为小写字符串,破坏原有数据
	extern char * firstUpper(char* szIn);      //转换为首字母小写字符串,破坏原有数据
  extern char * trimLeft(char *pStr);       	//裁减掉字符串左边的空格,破坏原有数据
	extern char * trimRight(char *pStr);      	//裁减掉字符串右边的空格,破坏原有数据
	extern char * trim(char *pStr);           	//裁减掉字符串首尾两头的空格,破坏原有数据
	
	/*模拟ORACLE的like， 当前支持%多字符通配符, '_'单字符通配符,'%'表示成'\%','\'表示成'\\','_'表示成'\_' 	*/
	extern bool like(const char * pSrc, const char * pTag);
	//用串pDescStrr来替换pStrBuf串中的pSrcSt子串,pStrBuf必须足够大，不安全
	extern char * replace(char *pStrBuf,const char *pSrcStr,const char *pDescStr);

  //毫秒级随眠
  extern void uSleep(int num);
  //动态装载链接库中的函数,仅支持linux/unix平台，注意并未调用close函数，仅适用于启动open后就不再close的场景
  extern void * loadDllFunc(const char *pLib,const char *pModule);
  //金额转换成汉字
  extern char * moneyToCapital(int amount,char * res_str=NULL);
  //字符转换
	//extern char * strToHex(const char * p_str,char * res_buf,int size);
	//extern char * hexToStr(const char * p_hex,char * res_buf,int size);

}

#endif

