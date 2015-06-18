/****************************************************************************
** ** Program: This file is part of the Util                 
** Author:  leiqiang  
** Date  :  2006-02-01
**
** RCS: $Id: sss.h,v 1.1.1.1 2006/09/25 01:28:12 sss Exp $
**
****************************************************************************/




#ifndef PropConfig_H
#define PropConfig_H

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <list>

using namespace std;


class Property
{
public:
	 string key;
	 string value;
	 Property& operator=(const Property& p)
		{
			if( this != &p )
			{
				key = p.key;
				value = p.value;
			}
		return *this;
	  }
};

/*================Example====================*
#############################################################
#                  For Kado Code Debug Logger
#############################################################
log4j.logger.com.wasu=DEBUG, C
log4j.appender.C=org.apache.log4j.RollingFileAppender 
log4j.appender.C.File=logs/gp-kado-code.log 
log4j.appender.C.MaxFileSize=5MB
log4j.appender.C.MaxBackupIndex=12
*================Example====================
注意，只有开头为'#'理解为注释，其他任何地方出现'#'也理解为正文
log4j.appender可看做log4j.appender.C的父节点
*/

class PropConfig
{
    public:
        PropConfig(){};
        bool    init(const char* pConfFile);  
        //按key获取Value，例如log4j.appender.C.File
				string  getValue(const char * key);
				string  getValue(const string &key);
				//按key的前缀，获取符合前缀的所有值，例如log4j.appender
				bool    find(const char *front_str,list<Property>  &props);
				bool    find(const string &front_str,list<Property>  &props);
    private:
        map<string,Property>  Props;
        bool     parse(const char * buf,Property &prop);
};


#endif

