#include"net_protocol.h"
#include "ctl_daemon_util.h"
void* arb(void *arg)
{
	fd_set readset;
	cmdToken ctoken;
	printids("arb thread is \n");
	pipeInOut *pipe=(pipeInOut*)arg;
	while(1)    	
	{
		FD_ZERO(&readset);
		FD_SET(pipe->pi,&readset);
		switch(select(pipe->pi+1,&readset,NULL,NULL,NULL))
		{
			case -1:
				perror("selct3 error");
				break;
			case 0:
				break;
			default:
			if (FD_ISSET(pipe->pi,&readset))
			{	
				read(pipe->pi,&ctoken,sizeof(cmdToken));
				printf("arb Freq is %f\n",ctoken.token.Value2);
				ctoken.token.Value2+=0.01;
				write(pipe->po,&ctoken,sizeof(cmdToken));
				printf("return\n");
			}
		}

	}

	return (void*)0;
}
