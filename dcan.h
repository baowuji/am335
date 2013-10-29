//
//Code for LSCM - can.h
//Copyright (C) 2013 SIBET. - http://www.sibet.ac.cn/
//Created by Gao Fei in February 5, 2013
//

#ifndef	DCAN_H
#define	DCAN_H
#define CMD_SET 0x00
#define CMD_GET 0x01
#define STATUS_SET_RET 0x00
#define STATUS_GET_RET 0x01
#define CANADDR_BEAGLEBONE 0x0F
#define CANADDR_LASER      0x00
#define CANADDR_SLIT       0x02
#define CANADDR_PINHOLE    0x03
#define CANADDR_DICHROIC   0x04
#define CAN_DEV_MAX_NUM 16 
//It's depends on the electroics parameters of CAN network,here 16 is just a pessmistic number which is enough in current system
typedef struct{
	unsigned int can_id;
	unsigned int can_mask;
}canFilter;

void*candaemon(void*arg);
int can_config(int argc,char*argv[]);
int can_start(int can_index, char *can_bitrate);

int can_stop(int can_index);

int can_send_sff(int can_index, unsigned int can_id,unsigned char can_dlc, unsigned char *can_data);

int can_recv_filter(int can_index,int filterNum,canFilter *filters,void*arg); 

unsigned int canidGen(unsigned char cmd,unsigned char dest,unsigned char source);
unsigned char getCanAddr(unsigned int canId);
void can_init(void);
#endif

