#include"net_protocol.h"
#include "ctl_daemon_util.h"
#include"arb.h"
#include"aotf.h"


int main(void)
{
	int listenfd,connfd;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen;
	unsigned int port=8722;
	int maxi,maxfd,sockfd;
	int nready,client[FD_SETSIZE];
	int i,tmp;
	int n;//return value of read
	netToken token;
	cmdToken ctoken;

	fd_set rset,wset;
	fd_set allset;
	struct timeval selTime;
	pipeInOut Paotf,Parb;

	int pipe_aotf[2],pipe_arb[2];
	int pipe_aotfO[2],pipe_arbO[2];

	if(pipe(pipe_aotf)<0)
		perror("pipe error\n");
	if(pipe(pipe_arb)<0)
		perror("pipe error\n");
	if(pipe(pipe_aotfO)<0)
		perror("pipe error\n");
	if(pipe(pipe_arbO)<0)
		perror("pipe error\n");

	Paotf.pi=pipe_aotf[0];Paotf.po=pipe_aotfO[1];
	Parb.pi=pipe_arb[0];Parb.po=pipe_arbO[1];

	pthread_t t_aotf,t_arb;
	pthread_create(&t_aotf,NULL,aotf,(void*)&Paotf);
	pthread_create(&t_arb,NULL,arb,(void*)&Parb);
	//
	printf("create thread ok\n");
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

	FD_ZERO(&wset);
	FD_SET(pipe_arbO[0],&allset);
	FD_SET(pipe_aotfO[0],&allset);

	maxfd=bigger(maxfd,bigger(pipe_arbO[0],pipe_aotfO[0]));
	while(1)
	{
		rset=allset;
		selTime.tv_sec=5;//linux will change the timeval in select ,so it's necessary to initialize it every time.in POSIX ,it's const 
		selTime.tv_usec=0;
		// 	nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		nready=select(maxfd+1,&rset,&wset,NULL,&selTime);

		//handle output here
		if(FD_ISSET(pipe_aotfO[0],&rset))
		{
			device_echo(pipe_aotfO[0],client);  		
			nready--;
		}
		if(FD_ISSET(pipe_arbO[0],&rset))
		{
			device_echo(pipe_arbO[0],client);  		
			nready--;
		}
		printf("----\n");	


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
							ctoken.sockfd=sockfd;
							ctoken.token=token;
							write(pipe_aotf[1],&ctoken,sizeof(cmdToken));
							break;
						case HD_XYSCANNER:
							ctoken.sockfd=sockfd;
							ctoken.token=token;
							write(pipe_arb[1],&ctoken,sizeof(cmdToken));
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

