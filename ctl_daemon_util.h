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

void printids(const char *s);




int device_echo(int pipeDes,int client[]);

int bigger(int a,int b);
#endif
