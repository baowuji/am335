#include"net_protocol.h"
#include "ctl_daemon_util.h"
#include     <stdio.h>
#include     <stdlib.h> 
#include     <unistd.h>  
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h> 
#include     <termios.h>
#include     <errno.h>
#include    <math.h>
#include    <string.h>

#define uartCmdLen 30
void* arb(void *arg)
{
	int fd;
	int i,n;
	char write_buf[uartCmdLen];
	struct termios opt;
	fd_set readset;
	cmdToken ctoken;
	printids("arb thread is \n");
	pipeInOut *pipe=(pipeInOut*)arg;

	////////////open uart port
	fd = open("/dev/ttyO2", O_RDWR | O_NOCTTY);  
	if(fd == -1) 
	{   
		perror("open serial 0\n");
		exit(0);
	}   

	tcgetattr(fd, &opt);    
	cfsetispeed(&opt, B115200);
	cfsetospeed(&opt, B115200);

	if(tcsetattr(fd, TCSANOW, &opt) != 0 ) 
	{    
		perror("tcsetattr error");
		return -1; 
	}   

	opt.c_cflag|=CS8;
	opt.c_cflag&=~CSTOPB;

	tcflush(fd, TCIOFLUSH);

	printf("configure complete\n");

	if(tcsetattr(fd, TCSANOW, &opt) != 0)
	{   
		perror("serial error");
		return -1;
	}
	printf("uart start send and receive data\n");

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

					bzero(write_buf, sizeof(write_buf));
					for(i=0;i<uartCmdLen;i++)
						write_buf[i]=ctoken.token.Val[i]; 
					n=write(fd,write_buf,uartCmdLen); 
					printf("%d bits arb param send to Arb\n",n);
				}
		}

	}

	return (void*)0;
}
