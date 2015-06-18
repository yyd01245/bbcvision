#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <iostream>



#include "TimeDelayParser.h"


//#include <my_basetype.h>

static int g_single_proc_inst_lock_fd = -1;

static void single_proc_inst_lockfile_cleanup(void)
{
    if (g_single_proc_inst_lock_fd != -1) {
        close(g_single_proc_inst_lock_fd);
        g_single_proc_inst_lock_fd = -1;
    }
}

bool is_single_proc_inst_running(const char *process_name)
{
	printf("-----into single \n");
    char lock_file[128];
    snprintf(lock_file, sizeof(lock_file), "/var/tmp/%s.lock", process_name);

    g_single_proc_inst_lock_fd = open(lock_file, O_CREAT|O_RDWR, 0644);
    if (-1 == g_single_proc_inst_lock_fd) {
        printf("Fail to open lock file(). Error: \n");
        return false;
    }

    if (0 == flock(g_single_proc_inst_lock_fd, LOCK_EX | LOCK_NB)) {
		printf("----succes lock\n");
		atexit(single_proc_inst_lockfile_cleanup);
        return true;
    }

	printf("can't lock file.\n");
    close(g_single_proc_inst_lock_fd);
    g_single_proc_inst_lock_fd = -1;
    return false;

}


//

int main(int argc ,char **argv)
{

#if 0
	signal(SIGINT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGFPE, SIG_IGN);
	signal(SIGSEGV, SIG_IGN);

	if(!is_single_proc_inst_running(argv[0]))
	{
		printf("---has pid \n");
		fflush(stdout);
		return -1;
	}
#endif

	char config_file[500];
	int ret;
/*   
   if(argc != 5 )
   {
	   fprintf(stderr,"please input Right param ,format example: ./TDP port=12345 127.0.0.1 25000 wwww.baidu.com");
	   return -1;
   }
*/
   
   TimeDelayParser *pTDP = new TimeDelayParser;

	if(argc == 5)
	{
	   if(pTDP->init(argv[1],argv[2],argv[3],argv[4]) < 0 )
	   	{
			fprintf(stderr,"please input Right param ,format example: ./TDP port=12345");
			return -1;
	   	}
	}
	else
	{
		if(pTDP->init(argv[1]) < 0 )
		 {
			 fprintf(stderr,"please input Right param ,format example: ./TDP port=12345");
			 return -1;
		 }


	}
	
	while(1)
	{
		sleep(10000);
	}
	printf("----go to switch \n");
	
	return 0;

}
