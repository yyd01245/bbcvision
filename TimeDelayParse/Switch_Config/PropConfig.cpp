
#include "PropConfig.h"
#include "Pub_c.h"

bool  PropConfig::parse(const char * data,Property &prop)
{
	  char buf[2048],tmp_buf[1024],*pos;
	  strncpy(buf,data,sizeof buf);
	  Pubc::trimLeft(buf);
	  if(buf[0]=='#' or buf[0]=='/') return false;//ÊÇ×¢ÊÍ
	  if((pos=strstr(buf,"="))==NULL) return false;//±íÊ¾Ê§°Ü
	  *pos='\0';
	  strcpy(tmp_buf,buf);
	  Pubc::trim(tmp_buf);
	  prop.key=tmp_buf;
	  
	  strcpy(tmp_buf,pos+1);
	  Pubc::trim(tmp_buf);
	  prop.value=tmp_buf;
	  
	  return true;
}

bool PropConfig::init(const char* pConfFile)
{
	FILE *fp;
	char buf[2048];
	Property prop;
	if((fp=fopen(pConfFile,"r"))==NULL)
		return  false; 
	
	while(!feof(fp))
	{
		 if(fgets(buf,2000,fp)==NULL)  break; 
		 if(parse(buf,prop)) 
		 		Props[prop.key]=prop;
	}
	fclose(fp);
	return true;
}


string PropConfig::getValue(const char * key)
{
	string s_key=key;
  return getValue(s_key);
}

string PropConfig::getValue(const string &key)
{
	map<string,Property>::iterator it=Props.find(key);
  if(it!=Props.end())
  	return it->second.value;
  else
  	return string("");
}

bool   PropConfig::find(const char *front_str,list<Property>  &props)
{
	string s_key=front_str;
  return find(s_key,props);
}

bool   PropConfig::find(const string &p_front_str,list<Property>  &props)
{
	 for(map<string,Property>::iterator it=Props.begin();
	 	   it!=Props.end();it++)
	 {
	 	 if(memcmp(it->first.c_str(),p_front_str.c_str(),p_front_str.length())==0)
	 	 	  props.push_back(it->second);
	 }
}


