#include "bcsp_work_admin.h"

int Bcsp_Admin_Handler::handle_process()
{
	char req_buf[2048];
	int	i_rc=0,i_count=0;
	string ret_string;	
	string host;
	int port;
	char* cmd=NULL;
	//step1:recv data
	do
	{
		i_rc=read(req_buf+i_count,2000-i_count,1,1);
	 	if(i_rc<=0)break;//异常关闭
	 		i_count+=i_rc;			
	}while(strstr(req_buf,"XXEE")==NULL);	 	  
	if(i_count >0 ) 
		req_buf[i_count]='\0';
	else
		return -1;//异常处理
	get_peer(host,port);
	
	LOG_INFO_FORMAT("INFO  - [BCSP]: tcp recv [%s:%d] thread=%lu socketid=%d ,recved %d bytes :[%s] \n",host.c_str(),port,pthread_self(),getHandle(),i_count,req_buf);

	//step2:parse data
	if(!strstr(req_buf,"XXEE")) return -1;
	Pubc::replace(req_buf,"XXEE","");
	cJSON *root = cJSON_Parse(req_buf);
	get_status(root, ret_string);
	cJSON_Delete(root);
	write(ret_string.c_str(),ret_string.size(),2);
	root=NULL;
	LOG_INFO_FORMAT("INFO  -[BCSP]: tcp send [%s:%d] %s\n",host.c_str(),port,ret_string.c_str());
	return -1;
}

bool Bcsp_Admin_Handler::get_status(cJSON *root,string &ret_string)
{
	cJSON *cmd,*ret_root,*bcchannels;
//	cJSON *channel[MAX_CHANNEL];
	list<std::string>::iterator iter;
	string uuid;
	char buf[1024];
	int i=0;

	cJSON *serialno = cJSON_GetObjectItem(root,"serialno");
	if(cmd = cJSON_GetObjectItem(root,"cmd"))
	{
		ret_root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"cmd","bclist");
		cJSON_AddStringToObject(root,"retcode","0");
	#if 0
		cJSON_AddItemToObject(root,"bcchannels",bcchannels = cJSON_CreateArray());
	
		for(iter = G_UUIDS_sngton::instance()->uuids.begin();
			iter != G_UUIDS_sngton::instance()->uuids.end();
			iter++)
		{
			cJSON *channel;
		//	JSON_create(*iter);
		//	JSON_send();
		//	JSON_parse();
			uuid = *iter;
/*			cJSON_AddItemToArray(bcchannels,channel[i] = cJSON_CreateObject());
			cJSON_AddStringToObject(channel[i],"name",uuid.c_str());
			cJSON_AddStringToObject(channel[i],"status","RUNNING");
			cJSON_AddNumberToObject(channel[i],"bitrate",3750000);
			i++;
*/
			cJSON_AddItemToArray(bcchannels,channel = cJSON_CreateObject());
			cJSON_AddStringToObject(channel,"name",uuid.c_str());
			cJSON_AddStringToObject(channel,"status","RUNNING");
			cJSON_AddNumberToObject(channel,"bitrate",3750000);

		}
		#endif
		char * m_tmp;
		m_tmp = cJSON_Print(ret_root);
		cJSON_AddItemToObject(ret_root,"serialno",serialno);
		sprintf(buf,"%sXXEE",m_tmp);
		free(m_tmp);
		ret_string = buf;
		cJSON_Delete(ret_root);
	}
	return true;
}
bool Bcsp_Admin_Handler::JSON_create(string &uuid,string &ret_string)
{
	return true;
}

bool Bcsp_Admin_Handler::JSON_send(string JSON_send,string &JSON_recv)
{
	return true;
}

bool Bcsp_Admin_Handler::JSON_parse(string &JSON_recv)
{
	return true;
}


