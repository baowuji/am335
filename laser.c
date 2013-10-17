#include"net_protocol.h"
#include "ctl_daemon_util.h"
#include"can.h"
#define LASERNUM 4
#define LASERCTLWORD LASERNUM*2
#define RSL 32 //return status length
void can_init(void)

{
	    //  can_stop(0);
	    can_stop(1);
		    //  can_start(0, CAN_BITRATE);
		    can_start(1, "50000");
}
void* laserCtl(void *arg)
{
	int i;
	fd_set readset;
	char returnStatus[RSL];
	char laserCtlWord[LASERCTLWORD];
	unsigned char recv_dlc;
	unsigned int recv_id;
	unsigned char recv_data[8] ;
	cmdToken ctoken;
	pipeInOut *pipe=(pipeInOut*)arg;
	printids("Laser Control\n");

	can_init();

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
						laserCtlWord[i]=ctoken.token.Val[i];
					//Add can send message here
					can_send_sff(1,0x3c0,8,laserCtlWord);
					can_recv_filter(1,0x01E,0x7ff,&recv_id,&recv_dlc,recv_data);
					//receive feedback from laser controller
					printf("laser setup\n");
				}
		}

	}
	return (void*)0;
}
