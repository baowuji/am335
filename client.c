#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include"client_arb.c"
#include"net_protocol.h"
void str_cli(FILE *fp, int sockfd);
int main(int argc, char **argv)
{
   int sockfd;
   int port = 8722;
   int i;
   int result;
  	double fastFreq=100;			 //振镜快轴频率
	double slowFreq=0.2;			 //振镜慢轴频率
	float fastScale=2,slowScale=2;
	double biasX=0,biasY=0;//X,Y轴的初始偏移offset
	char slowWaveForm=3,fastWaveForm=3;//扫描波形，SINE：0，TRI：2，RAMP：3，DC:14
	int row_in_frame_number=500;		 //一帧扫描的行数
	double cycle=5.1;			         //一帧扫描时间
	int sample_num=500;	             //每行的采样点数
	int ivc102_T1=800;		 //T1为积分是时间
	int ivc102_T2=850;			 //T2为保持时间
	int ivc102_T3=890;		 //T3为产生采样上升沿的时间之后一直为高
	int ivc102_T4=950;		 //T4为放电时间
	double sampleFreq=51;		 //采样频率，由于使用一个字节传输，最大数255，因此单位为Khz

	char uartCom[31];

   struct sockaddr_in servaddr;
   netToken token = { 'S', HD_ARB,CMD_SET,"",'E' };
   netToken tokenIn;
   arbGen(uartCom,fastFreq,slowFreq,fastScale,slowScale,
		   biasX,biasY,slowWaveForm,fastWaveForm,row_in_frame_number,
		   cycle,sample_num,ivc102_T1,ivc102_T2,ivc102_T3,ivc102_T4,sampleFreq);
   for(i=0;i<31;i++)
	   token.Val[i]=uartCom[i];
   if (argc != 2)
   {
      printf("client ip \n");
      exit(1);
   }
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      printf("socket create error\n");
      exit(1);
   }
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(port);
   inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
   result = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
   if (result == -1)
	{ 
      perror("connect error!\n");
      exit(1);
	}
	i=1;
	printf("ok\n");
	printf("%s\n",uartCom);
   while (i-->0)
   {
     write(sockfd, &token, sizeof(token));
	 if(token.Device==HD_LASER)
		 token.Device=HD_ARB;
	 else
		 token.Device=HD_LASER;
     sleep(1);
	 printf("send once\n");
   }

//   shutdown(sockfd,SHUT_RDWR);
   exit(0);
}
