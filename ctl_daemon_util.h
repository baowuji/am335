#ifndef _CTL_DAEMON_UTIL_H
#define _CTL_DAEMON_UTIL_H

#include"net_protocol.h"
#include<pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>



typedef struct{
	int sockfd;
	netToken token;
}cmdToken;
typedef struct{
	int pi;
	int po;
}pipeInOut;

void printids(const char *s)
{
	pid_t	pid;
	pthread_t tid;
	pid=getpid();
	tid=pthread_self();
	printf("%s pid %u tid %u (0x%x)\n",s,(unsigned int)pid,(unsigned int )tid,(unsigned int)tid);
}




int device_echo(int pipeDes,int client[])
{
	int i;
	cmdToken ctoken;
	
	read(pipeDes,&ctoken,sizeof(cmdToken));
	for(i=0;i<FD_SETSIZE;i++)
	{

		if(ctoken.sockfd==client[i])
		{
			write(client[i],&ctoken.token,sizeof(netToken));
			break;
		}
	}
	if (i==FD_SETSIZE)
	{
		printf("%d closed before status return\n",ctoken.sockfd);
		return -1;
	}

	
	return 0;
}
int bigger(int a,int b)
{
	return a>b?a:b;
}
#endif
