

#include "Record.h"

//-----------------------------------Record--------------------------------------------
bool    Record::set(int index,string &value)
{
    try
    {   
        if(index<0) 
            return false;
        if(index>=size()) 
            resize(index+1);
        m_listValues[index]=value;
        return true;
    }
    catch(exception &e)
    {
        cout<<__LINE__<<e.what()<<endl;
        return false;
    }
}

bool    Record::set(int index,const char* szValue)
{
    string val=szValue;
    return set(index,val);
}

bool    Record::get(int index,string &value) const
{
    try
    {   
        if(index<0||index>=size()) 
            return false;
        value=m_listValues[index];
        return true;
    }
    catch(exception &e)
    {
        cout<<__LINE__<<e.what()<<endl;
        return false;
    }
}

bool    Record::get(int index,int &value) const
{
    value=0;
    string str;
    if(get(index,str))
    {
    	value=atoi(str.c_str());
    	return true;
    }
    return false;
}

bool Record::isvalid_addr(string &value) const
{

	int ret = -1;
	unsigned int ip1,ip2,ip3,ip4;
	char port_s[8];
	memset(port_s,0,sizeof(port_s));
	
	ret = sscanf(value.c_str(),"%u.%u.%u.%u:%s",&ip1,&ip2,&ip3,&ip4,port_s);
	if(ret != 5 || ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255)
		return false;
	
	char *port_tmp = port_s;
		
	while(*port_tmp)
	{
		if(*port_tmp >'9' || *port_tmp < '0')
			return false;
		port_tmp++;
	}

	if(atoi(port_s) > 65535)
		return true;
	
	return true;
}

bool Record::issHexNumber(const char *pBuf)
{
		int i = 0;
		int bufSize = strlen(pBuf);
		if (bufSize >	32) return false;
    for (i = 0; i < bufSize; ++i)
    {
        if (('0' > pBuf[i] || '9' < pBuf[i])
            && ('A' > pBuf[i] || 'F' < pBuf[i])
            && ('a' > pBuf[i] || 'f' < pBuf[i]))
        {
            return false;
        }
    }
    return true;
}


Record&   Record::operator=(const Record&rec)
{
    if( this != &rec )
	{
		m_listValues=rec.m_listValues;
	}
	return *this;
}

bool Record::from_String(const char *str,const char* strflag)
{
    try
    {   
        int i=0,len;
        char buf[1024+1];
        const char *p=strstr(str,strflag);
        clear();
        while(p!=NULL)
        {
            len = p - str;
            if(len==0)
                set(i,"");
            else
            {
                if(len>1024)
                    return false;
                memcpy(buf,str,len);
                buf[len]='\0';
                Pubc::trim(buf);
                set(i,buf);
            }
            str=p+1;
            p=strstr(str,strflag);
            i++;
        }
        strcpy(buf,str);
        Pubc::trim(buf);
        set(i,buf);
        return true;
    }
    catch(exception &e)
    {
        cout<<__LINE__<<e.what()<<endl;
        return false;
    }
}


string Record::to_string()
{
	string str;
	char buf[1024];
	sprintf(buf,"total:%d  \n",size());
  str+=buf;
  for(int i=0;i<size();i++)
	{	 
		 sprintf(buf," %d=[%s];",i,m_listValues[i].c_str());
		 str+=buf;
	} 
	return str;
}

//----------------------------END Record----------------------------------
