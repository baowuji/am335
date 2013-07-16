#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include"net_protocol.h"
#define TOKEN_SIZE 22
void str_cli(FILE *fp, int sockfd);
int main(int argc, char **argv)
{
   int sockfd;
   int port = 8722;

   int result;
   struct sockaddr_in servaddr;
   netToken token = { '#', FUNC_GET, HD_AOTF, FirstSpectral_Blue, 1.0, 2.2 };
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

   while (1)
   {
      write(sockfd, &token, sizeof(token));
	 if(token.Device==HD_AOTF)
		 token.Device=HD_XYSCANNER;
	 else
		 token.Device=HD_AOTF;
     token.Value2 += 1;
     sleep(1);
   }

//   shutdown(sockfd,SHUT_RDWR);
   exit(0);
}
void str_cli(FILE *fp, int sockfd)
{
}
