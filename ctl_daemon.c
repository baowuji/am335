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
int pipe_aotf[2],pipe_arb[2];
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
	fd_set readset;

	netToken token;
	printids("aotf\n");
	while(1)    	
	{
		FD_ZERO(&readset);
		FD_SET(pipe_aotf[0],&readset);
		switch(select(pipe_aotf[0]+1,&readset,NULL,NULL,NULL))
		{
			case -1:
				perror("selct2 error");
				break;
			case 0:
				break;
			default:
			if (FD_ISSET(pipe_aotf[0],&readset))
			{		read(pipe_aotf[0],&token,sizeof(netToken));
				printf("aotf Freq is %f\n",token.Value2);
			}
		}

	}
	return (void*)0;
}
void* arb(void *arg)
{
	fd_set readset;

	netToken token;
	printids("arb thread is \n");
	while(1)    	
	{
		FD_ZERO(&readset);
		FD_SET(pipe_arb[0],&readset);
		switch(select(pipe_arb[0]+1,&readset,NULL,NULL,NULL))
		{
			case -1:
				perror("selct3 error");
				break;
			case 0:
				break;
			default:
			if (FD_ISSET(pipe_arb[0],&readset))
			{	
				read(pipe_arb[0],&token,sizeof(netToken));
				printf("arb Freq is %f\n",token.Value2);
			}
		}

	}

	return (void*)0;
}

int main(void)
{
	int listenfd,connfd;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen;
	unsigned int port=8722;
	int maxi,maxfd,sockfd;
	int nready,client[FD_SETSIZE];
	int i;
	int n;//return value of read
	netToken token;
	fd_set rset;
	fd_set allset;
	struct timeval selTime;



	if(pipe(pipe_aotf)<0)
		perror("pipe error\n");

	if(pipe(pipe_arb)<0)
		perror("pipe error\n");

	pthread_t t_aotf,t_arb;
	pthread_create(&t_aotf,NULL,aotf,(void*)1);
	pthread_create(&t_arb,NULL,arb,(void*)1);
//
	if((listenfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
	{
		perror("socket creation error!\n");
		exit(1);
	}

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
	  
	maxfd=listenfd;
	maxi=-1;
	for(i=0;i<FD_SETSIZE;i++)
		client[i]=-1;
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);
	while(1)
	{
		rset=allset;
		selTime.tv_sec=5;//linux will change the timeval in select ,so it's necessary to initialize it every time.in POSIX ,it's const 
    	selTime.tv_usec=0;
    	nready=select(maxfd+1,&rset,NULL,NULL,&selTime);
		
		write(0,"---\n",5);
		if(FD_ISSET(listenfd,&rset))//new client connection
		{
			clilen=sizeof(cliaddr);
			if((connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen))==-1)
			{
				perror("accept error!\n");
			}
	
			for(i=0;i<FD_SETSIZE;i++)
				if(client[i]<0)
				{
					client[i]=connfd;//save descriptor
					break;
				}
			if(i==FD_SETSIZE)
				perror("Too Many clients");
			FD_SET(connfd,&allset);//add new descriptor to set
			if(connfd>maxfd)
				maxfd=connfd;//for select
			if(i>maxi)
				maxi=i;//max index of client[]
			if(--nready<=0)
				continue;
		}

		for(i=0;i<=maxi;i++)//check all clients for data
		{
			if((sockfd=client[i])<0)
				continue;
			if(FD_ISSET(sockfd,&rset))
			{
    			if((n=read(sockfd,&token,sizeof(netToken)))==0)//connection closed by client
				{
					close(sockfd);
					FD_CLR(sockfd,&allset);
					client[i]=-1;
				}
				else
				{
					switch(token.Device)
    				{
    					case HD_AOTF:
    						write(pipe_aotf[1],&token,sizeof(netToken));
    						break;
    					case HD_XYSCANNER:
    						write(pipe_arb[1],&token,sizeof(netToken));
    						break;
    					default:
    						;
    				}
				}
			if(--nready<=0)//no more readable descriptors
				break;
    		}	
    	
    	}
	}
	exit(0);
}
