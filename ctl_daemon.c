#include"apue.h"
#include"unp.h"
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

#define TOKEN_SIZE 22
typedef struct {
	int a,b;
}NetData;
typedef NetData* pNetData;
NetData net1;
pNetData pnet1=&net1;
typedef struct 
{
	int freq;
	int power;
}AotfData;
typedef AotfData* pAotfData;
AotfData aotf1={1,1};
pAotfData paotf1=&aotf1;
void set_net();
void printids(const char *s)
{
	pid_t	pid;
	pthread_t tid;
	pid=getpid();
	tid=pthread_self();
	printf("%s pid %u tid %u (0x%x)\n",s,(unsigned int)pid,(unsigned int )tid,(unsigned int)tid);
}
void* aotf(void *arg)
{
	printids("aotf\n");
//	while(1)    	
	{
		printf("aotf Freq is %d\n",aotf1.freq);
	}
	return (void*)0;
}
void* arb(void *arg)
{
	printids("arb thread is \n");

	return (void*)0;
}
void handle_request(int sockfd)
{
	int n;
	netToken token;
	fd_set readset,writeset;
	struct timeval selTime;
	int ret;
	selTime.tv_sec=5;
	selTime.tv_usec=0;
	while(1)
	{
	FD_ZERO(&readset);
	FD_ZERO(&writeset);
	FD_SET(sockfd,&readset);
	
//	ret=select(sockfd+1,&readset,&writeset,NULL,&selTime);
	ret=select(sockfd+1,&readset,&writeset,NULL,NULL);
	switch(ret)
	{
		case -1:
			perror("select1 error\n");
			break;
		case 0:
//			printf("select1 time out\n");
			break;
		default:	
			printf("ret=%d\n",ret);
			if (FD_ISSET(sockfd,&readset))
			{
				if((n=read(sockfd,&token,sizeof(netToken)))>0)
				printf("%c %d %d %d %f %f\n",token.Device,token.Command,token.Type,token.Status,token.Value1,token.Value2);
			}	
	}
	}

}

int main(void)
{
	int listenfd,connfd;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen;
	unsigned int port=8722;
	int x;
	



	pthread_t t_aotf,t_arb;
	pthread_create(&t_aotf,NULL,aotf,(void*)1);
//	pthread_create(&t_arb,NULL,arb,(void*)1);
//
	if((listenfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
	{
		perror("socket creation error!\n");
		exit(1);
	}
//	x=fcntl(listenfd,F_GETFL,0);//get socket flags
//	fcntl(listenfd,F_SETFL,x|O_NONBLOCK);//add non-blocking flag

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(port);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr)) < 0)
	{
		perror( "Bind error.");
		exit(1);
	}
	if(listen(listenfd,5) == -1)
	{
		printf( "listen error!\n");
	    exit(1);
	}	
	  
	while(1)
	{
		clilen=sizeof(cliaddr);
		if((connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen))==-1)
		{
			perror("accept error!\n");
		}
		handle_request(connfd);
		close(connfd);
	}
	
	exit(0);
}