#include"net_protocol.h"
#include "ctl_daemon_util.h"
typedef struct 
{
	int freq;
	int power;
}AotfData;
void* aotf(void *arg)
{
	fd_set readset;

	cmdToken ctoken;
	pipeInOut *pipe=(pipeInOut*)arg;
	printids("aotf\n");
	while(1)    	
	{
		FD_ZERO(&readset);
		FD_SET(pipe->pi,&readset);
		switch(select(pipe->pi+1,&readset,NULL,NULL,NULL))
		{
			case -1:
				perror("selct2 error");
				break;
			case 0:
				break;
			default:
			if (FD_ISSET(pipe->pi,&readset))
			{	
				read(pipe->pi,&ctoken,sizeof(cmdToken));
				write(pipe->po,&ctoken,sizeof(cmdToken));
				printf("atof config return\n");
			}
		}

	}
	return (void*)0;
}
