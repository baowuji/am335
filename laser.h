#include"net_protocol.h"
#include "ctl_daemon_util.h"
#define LASERNUM 4
#define LASERCTLWORD LASERNUM*2

void* laserCtl(void *arg)
{
	int i;
	fd_set readset;

	char laserCtlWord[LASERCTLWORD];
	cmdToken ctoken;
	pipeInOut *pipe=(pipeInOut*)arg;
	printids("Laser Control\n");
	
	while(1)    	
	{
		FD_ZERO(&readset);
		FD_SET(pipe->pi,&readset);
		switch(select(pipe->pi+1,&readset,NULL,NULL,NULL))
		{
			case -1:
				perror("select @laser error");
				break;
			case 0:
				break;
			default:
				if (FD_ISSET(pipe->pi,&readset))
				{	
					read(pipe->pi,&ctoken,sizeof(cmdToken));
					for(i=0;i<LASERCTLWORD;i++)
						laserCtlWord[i]=cmdToken.Val[i];
					//Add can send message here
					can_send_sff(1,0x3c0,8,laserCtlWord);
					//receive feedback from laser controller
					printf("laser setup\n");
				}
		}

	}
	return (void*)0;
}
