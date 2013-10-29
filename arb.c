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

#define uartCmdLen 31
void* arb(void *arg)
{
	int fd;
	int i,n,maxfd;
	unsigned char write_buf[uartCmdLen];
	unsigned char read_buf[uartCmdLen];
	struct termios opt;
	fd_set readset;
	cmdToken ctoken;
	printids("arb thread is \n");
	pipeInOut *pipe=(pipeInOut*)arg;

	////////////open uart port
	fd = open("/dev/ttyO2", O_RDWR | O_NOCTTY | O_NDELAY );  
	if(fd == -1) 
	{   
		perror("open serial 0\n");
		exit(0);
	}   


	if(	fcntl(fd, F_SETFL, 0))//uart work in blocking mode
		{
			perror("fcntl in uart setting error\n");
			exit(0);
		}

	tcgetattr(fd, &opt);    
	cfsetispeed(&opt, B115200);
	cfsetospeed(&opt, B115200);

	if(tcsetattr(fd, TCSANOW, &opt) != 0 ) 
	{    
		perror("tcsetattr error");
		return (void*)-1; 
	}   


	opt.c_cflag &= ~PARENB;
	opt.c_cflag &= ~CSTOPB;
	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;

	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG );
			



	tcflush(fd, TCIOFLUSH);

	printf("configure complete\n");

	if(tcsetattr(fd, TCSANOW, &opt) != 0)
	{   
		perror("serial error");
		return (void*)-1;
	}
	printf("uart start send and receive data\n");

	maxfd=(pipe->pi)>fd?pipe->pi:fd;
	while(1)    	
	{
		FD_ZERO(&readset);
		FD_SET(pipe->pi,&readset);
		FD_SET(fd,&readset);
		switch(select(maxfd+1,&readset,NULL,NULL,NULL))
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
				else if(FD_ISSET(fd,&readset))
				{
					n=read(fd,read_buf,uartCmdLen);
					printf("%s\n",read_buf);
				}
		}

	}

	return (void*)0;
}
