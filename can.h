//
//Code for LSCM - can.h
//Copyright (C) 2013 SIBET. - http://www.sibet.ac.cn/
//Created by Gao Fei in February 5, 2013
//

#ifndef	CAN_H
#define	CAN_H

int can_start(int can_index, char *can_bitrate);

int can_stop(int can_index);

int can_send_sff(int can_index, unsigned int can_id,
		unsigned char can_dlc, unsigned char *can_data);

int can_recv_filter(int can_index, unsigned int can_id, unsigned int can_mask,
		unsigned int *recv_id,
		unsigned char *recv_dlc,
		unsigned char *recv_data);

#endif

