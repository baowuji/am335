#define LSCM_DEBUG
#define BUF_SIZ	(255)
#include <libsocketcan.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include"dcan.h"
#include "can_config.h"
#include "debug.h"
#include"ctl_daemon_util.h"
void can_init(void)
{
	can_stop(1);
	//  can_start(0, CAN_BITRATE);
	can_start(1, "50000");
}
unsigned int canidGen(unsigned char cmd,unsigned char dest,unsigned char source)
{
	return cmd|dest<<1|source<<6;
}
unsigned char getCanAddr(unsigned int canId)
{
	return (canId>>1)|0x001f;
}
//
//can_send_sff - send standard format frame
//@can_index: can index
//@can_id: can identifier, 11bits
//@can_dlc: data length, 1~8
//@can_data: data, 1~8bytes
//if successed return 0, else return 1
//
int can_send_sff(int can_index, unsigned int can_id,
		unsigned char can_dlc, unsigned char *can_data)
{
	struct can_frame frame = {
		.can_id = 1,
	};
	struct ifreq ifr;
	struct sockaddr_can addr;
	char *interface = "can0";//char *interface;
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
	int s, i, ret;

	//parameter check
	if ((can_index != 0) && (can_index != 1))
	{
		APP_ERROR("CAN INDEX NEEDS 0 OR 1.\n");
		return 1;
	}

	if ((can_dlc < 0) && (can_dlc > 8))
	{
		APP_ERROR("DATA LENGTH NEES 0~8.\n");
		return 1;
	}

	if (1 == can_index)
		interface = "can1";

	frame.can_id = can_id;
	frame.can_id &= CAN_SFF_MASK;

	s = socket(family, type, proto);
	if (s < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = family;
	strcpy(ifr.ifr_name, interface);
	if (ioctl(s, SIOCGIFINDEX, &ifr)) {
		perror("ioctl");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	frame.can_dlc = can_dlc;

	for (i = 0; i < can_dlc; i++)
		frame.data[i] = can_data[i];

	ret = write(s, &frame, sizeof(frame));
	if (ret == -1) {
		perror("write");
		return 1;
	}

	close(s);

	/*APP_INFOR("can_send_sff: %s <0x%03x> [%d]",
	  interface, frame.can_id, frame.can_dlc);
	  for (i = 0; i < can_dlc; i++)
	  APP_INFOR(" 0x%02x", frame.data[i]);
	  APP_INFOR("\n");*/

	return 0;
}

//
//Code for LSCM - candump.c
//Copyright (C) 2013 SIBET. - http://www.sibet.ac.cn/
//Edited by Gao Fei in February 21, 2013
//

static int	s = -1;

static struct can_filter *filter = NULL;
static int filter_count = 0;

int add_filter(u_int32_t id, u_int32_t mask)
{
	filter = realloc(filter, sizeof(struct can_filter) * (filter_count + 1));
	if(!filter)
		return -1;

	filter[filter_count].can_id = id;
	filter[filter_count].can_mask = mask;
	filter_count++;

	printf("id: 0x%08x mask: 0x%08x\n",id,mask);
	return 0;
}

//
//can_recv_filter - recv frame with filter
//@can_index: can index
//@can_id, can_mask: can identifier and mask
//@recv_id: recv frame identifier
//@recv_dlc: recv frame data length
//@recv_data: recv frame data
//if successed return 0, else return 1
//

static int running=1;
int can_recv_filter(int can_index, int filterNum,canFilter* filters,void*arg)
{
	struct can_frame frame;
	struct ifreq ifr;
	struct sockaddr_can addr;
	char *interface = "can0";
	char buf[BUF_SIZ];
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
	int n = 0;
	int nbytes, i;

	cmdToken ctoken;
	unsigned int recv_id;
	unsigned char recv_dlc; 
	unsigned char* recv_data=ctoken.token.Val;
	pipeInOut *pipe=(pipeInOut*)arg;
	printids("can daemon starts\n");


	//parameter check
	if ((can_index != 0) && (can_index != 1))
	{
		APP_ERROR("CAN INDEX NEEDS 0 OR 1.\n");
		return 1;
	}

	if (1 == can_index)
		interface = "can1";

	for(i=0;i<filterNum;i++)
		add_filter(filters[i].can_id,filters[i].can_mask); //each device matches one filter

	if ((s = socket(family, type, proto)) < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = family;
	strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFINDEX, &ifr)) {
		perror("ioctl");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (filter) {
		if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, filter,
					filter_count * sizeof(struct can_filter)) != 0) {
			perror("setsockopt");
			return 1;
		}
	}

	APP_INFOR("can_recv_filter: %s waiting......\n", interface);
	while(running){
		if ((nbytes = read(s, &frame, sizeof(struct can_frame))) < 0)
		{
			perror("read");
			return 1;
		}
		else
		{
			recv_id = frame.can_id;
			recv_dlc = frame.can_dlc;

			if (frame.can_id & CAN_EFF_FLAG)
				n = snprintf(buf, BUF_SIZ, "<0x%08x> ", frame.can_id & CAN_EFF_MASK);
			else
				n = snprintf(buf, BUF_SIZ, "<0x%03x> ", frame.can_id & CAN_SFF_MASK);

			n += snprintf(buf + n, BUF_SIZ - n, "[%d] ", frame.can_dlc);

			for (i = 0; i < frame.can_dlc; i++) {
				n += snprintf(buf + n, BUF_SIZ - n, "0x%02x ", frame.data[i]);
				recv_data[i] = frame.data[i];
			}

			if (frame.can_id & CAN_RTR_FLAG)
				n += snprintf(buf + n, BUF_SIZ - n, "remote request");

			APP_INFOR("can_recv_filter: %s %s\n", interface, buf);
			switch(getCanAddr(recv_id))
			{
				case CANADDR_LASER:
					ctoken.token.Device=HD_LASER;
					break;
				case CANADDR_SLIT:
					ctoken.token.Device=HD_SLIT;
					break;
				case CANADDR_DICHROIC:
					ctoken.token.Device=HD_DICHROIC;
					break;
				case CANADDR_PINHOLE:
					ctoken.token.Device=HD_PINHOLE;
					break;
				default:
					break;
			}
			write(pipe->po,&ctoken,sizeof(cmdToken));
			n = 0;
		}
	}

	return 0;
}

void* candaemon(void*arg)
{

	int can_index=1;
	unsigned int can_id;
	unsigned int can_mask=0x3fe;//receive the data the local controllers send without command
	int filterNum;
	canFilter laserF={
		.can_id=canidGen(CMD_SET,CANADDR_BEAGLEBONE,CANADDR_LASER),
		.can_mask=0x3fe};
	canFilter slitF={
		.can_id=canidGen(CMD_SET,CANADDR_BEAGLEBONE,CANADDR_SLIT),
		.can_mask=0x3fe};
	canFilter pinholeF={
		.can_id=canidGen(CMD_SET,CANADDR_BEAGLEBONE,CANADDR_PINHOLE),
		.can_mask=0x3fe};
	canFilter dichroicF={
		.can_id=canidGen(CMD_SET,CANADDR_BEAGLEBONE,CANADDR_DICHROIC),
		.can_mask=0x3fe};

	canFilter filters[CAN_DEV_MAX_NUM];
	//add filter of device here.
	filters[0]=laserF;
	filters[1]=slitF;
	filters[2]=pinholeF;
	filters[3]=dichroicF;
	filterNum=4;
	can_recv_filter(can_index,filterNum,filters,arg); 
	return;
}
//
//Code for LSCM - canconfig.c
//Copyright (C) 2013 SIBET. - http://www.sibet.ac.cn/
//Edited by Gao Fei in February 5, 2013
//

/*
 * canutils/canconfig.c
 *
 * Copyright (C) 2005, 2008 Marc Kleine-Budde <mkl@pengutronix.de>, Pengutronix
 * Copyright (C) 2007 Juergen Beisert <jbe@pengutronix.de>, Pengutronix
 * Copyright (C) 2009 Luotao Fu <l.fu@pengutronix.de>, Pengutronix
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

const char *can_states[CAN_STATE_MAX] = {
	"ERROR-ACTIVE",
	"ERROR-WARNING",
	"ERROR-PASSIVE",
	"BUS-OFF",
	"STOPPED",
	"SLEEPING"
};

const char *config_keywords[] = {
	"baudrate", "bitrate", "bittiming", "ctrlmode", "restart",
	"start", "stop", "restart-ms", "state", "clockfreq",
	"bittiming-const", "berr-counter"};

/* this is shamelessly stolen from iproute and slightly modified */
#define NEXT_ARG() \
	do { \
		argv++; \
		if (--argc < 0) { \
			fprintf(stderr, "missing parameter for %s\n", *argv); \
			break;\
		}\
	} while(0)

static inline int find_str(const char** haystack, unsigned int stack_size,
		const char* needle)
{
	unsigned int i, found = 0;

	for (i = 0; i < stack_size; i++) {
		if (!strcmp(needle, haystack[i])) {
			found = 1;
			break;
		}
	}

	return found;
}

static void help(void)
{
	fprintf(stderr, "usage:\n\t"
			"canconfig <dev> bitrate { BR } [sample-point { SP }]\n\t\t"
			"BR := <bitrate in Hz>\n\t\t"
			"SP := <sample-point {0...0.999}> (optional)\n\t"
			"canconfig <dev> bittiming [ VALs ]\n\t\t"
			"VALs := <tq | prop-seg | phase-seg1 | phase-seg2 | sjw>\n\t\t"
			"tq <time quantum in ns>\n\t\t"
			"prop-seg <no. in tq>\n\t\t"
			"phase-seg1 <no. in tq>\n\t\t"
			"phase-seg2 <no. in tq\n\t\t"
			"sjw <no. in tq> (optional)\n\t"
			"canconfig <dev> restart-ms { RESTART-MS }\n\t\t"
			"RESTART-MS := <autorestart interval in ms>\n\t"
			"canconfig <dev> ctrlmode { CTRLMODE }\n\t\t"
			"CTRLMODE := <[loopback | listen-only | triple-sampling | berr-reporting] [on|off]>\n\t"
			"canconfig <dev> {ACTION}\n\t\t"
			"ACTION := <[start|stop|restart]>\n\t"
			"canconfig <dev> clockfreq\n\t"
			"canconfig <dev> bittiming-constants\n\t"
			"canconfig <dev> berr-counter\n"
		   );

	//exit(EXIT_FAILURE);
	return;
}

static void do_show_bitrate(const char *name)
{
	struct can_bittiming bt;

	if (can_get_bittiming(name, &bt) < 0) {
		fprintf(stderr, "%s: failed to get bitrate\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else
		fprintf(stdout,
				"%s bitrate: %u, sample-point: %0.3f\n",
				name, bt.bitrate,
				(float)((float)bt.sample_point / 1000));
}

static void do_set_bitrate(int argc, char *argv[], const char *name)
{
	__u32 bitrate = 0;
	__u32 sample_point = 0;
	int err;

	while (argc > 0) {
		if (!strcmp(*argv, "bitrate")) {
			NEXT_ARG();
			bitrate =  (__u32)strtoul(*argv, NULL, 0);
		} else if (!strcmp(*argv, "sample-point")) {
			NEXT_ARG();
			sample_point = (__u32)(strtod(*argv, NULL) * 1000);
		}
		argc--, argv++;
	}

	if (sample_point)
		err = can_set_bitrate_samplepoint(name, bitrate, sample_point);
	else
		err = can_set_bitrate(name, bitrate);

	if (err < 0) {
		fprintf(stderr, "failed to set bitrate of %s to %u\n",
				name, bitrate);
		//exit(EXIT_FAILURE);
		return;
	}
}

static void cmd_bitrate(int argc, char *argv[], const char *name)
{
	int show_only = 1;

	if (argc > 0)
		show_only = find_str(config_keywords,
				sizeof(config_keywords) / sizeof(char*),
				argv[1]);

	if (! show_only)
		do_set_bitrate(argc, argv, name);

	do_show_bitrate(name);
}

static void do_set_bittiming(int argc, char *argv[], const char *name)
{
	struct can_bittiming bt;
	int bt_par_count = 0;

	memset(&bt, 0, sizeof(bt));

	while (argc > 0) {
		if (!strcmp(*argv, "tq")) {
			NEXT_ARG();
			bt.tq = (__u32)strtoul(*argv, NULL, 0);
			bt_par_count++;
			continue;
		}
		if (!strcmp(*argv, "prop-seg")) {
			NEXT_ARG();
			bt.prop_seg = (__u32)strtoul(*argv, NULL, 0);
			bt_par_count++;
			continue;
		}
		if (!strcmp(*argv, "phase-seg1")) {
			NEXT_ARG();
			bt.phase_seg1 = (__u32)strtoul(*argv, NULL, 0);
			bt_par_count++;
			continue;
		}
		if (!strcmp(*argv, "phase-seg2")) {
			NEXT_ARG();
			bt.phase_seg2 =
				(__u32)strtoul(*argv, NULL, 0);
			bt_par_count++;
			continue;
		}
		if (!strcmp(*argv, "sjw")) {
			NEXT_ARG();
			bt.sjw =
				(__u32)strtoul(*argv, NULL, 0);
			continue;
		}
		argc--, argv++;
	}
	/* kernel will take a default sjw value if it's zero. all other
	 * parameters have to be set */
	if (bt_par_count < 4) {
		fprintf(stderr, "%s: missing bittiming parameters, "
				"try help to figure out the correct format\n",
				name);
		//exit(1);
		return;
	}
	if (can_set_bittiming(name, &bt) < 0) {
		fprintf(stderr, "%s: unable to set bittiming\n", name);
		//exit(EXIT_FAILURE);
		return;
	}
}

static void do_show_bittiming(const char *name)
{
	struct can_bittiming bt;

	if (can_get_bittiming(name, &bt) < 0) {
		fprintf(stderr, "%s: failed to get bittiming\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else
		fprintf(stdout, "%s bittiming:\n\t"
				"tq: %u, prop-seq: %u phase-seq1: %u phase-seq2: %u "
				"sjw: %u, brp: %u\n",
				name, bt.tq, bt.prop_seg, bt.phase_seg1, bt.phase_seg2,
				bt.sjw, bt.brp);
}

static void cmd_bittiming(int argc, char *argv[], const char *name)
{
	int show_only = 1;

	if (argc > 0)
		show_only = find_str(config_keywords,
				sizeof(config_keywords) / sizeof(char*),
				argv[1]);

	if (! show_only)
		do_set_bittiming(argc, argv, name);

	do_show_bittiming(name);
	do_show_bitrate(name);
}

static void do_show_bittiming_const(const char *name)
{
	struct can_bittiming_const btc;

	if (can_get_bittiming_const(name, &btc) < 0) {
		fprintf(stderr, "%s: failed to get bittiming_const\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else
		fprintf(stdout, "%s bittiming-constants: name %s,\n\t"
				"tseg1-min: %u, tseg1-max: %u, "
				"tseg2-min: %u, tseg2-max: %u,\n\t"
				"sjw-max %u, brp-min: %u, brp-max: %u, brp-inc: %u,\n",
				name, btc.name, btc.tseg1_min, btc.tseg1_max,
				btc.tseg2_min, btc.tseg2_max, btc.sjw_max,
				btc.brp_min, btc.brp_max, btc.brp_inc);
}

static void cmd_bittiming_const(const char *name)
{
	do_show_bittiming_const(name);
}

static void do_show_state(const char *name)
{
	int state;

	if (can_get_state(name, &state) < 0) {
		fprintf(stderr, "%s: failed to get state \n", name);
		//exit(EXIT_FAILURE);
		return;
	}

	if (state >= 0 && state < CAN_STATE_MAX)
		fprintf(stdout, "%s state: %s\n", name, can_states[state]);
	else
		fprintf(stderr, "%s: unknown state\n", name);
}

static void cmd_state( const char *name)
{
	do_show_state(name);
}

static void do_show_clockfreq(const char *name)
{
	struct can_clock clock;

	memset(&clock, 0, sizeof(struct can_clock));
	if (can_get_clock(name, &clock) < 0) {
		fprintf(stderr, "%s: failed to get clock parameters\n",
				name);
		//exit(EXIT_FAILURE);
		return;
	}

	fprintf(stdout, "%s clock freq: %u\n", name, clock.freq);
}

static void cmd_clockfreq(const char *name)
{
	do_show_clockfreq(name);
}

static void do_restart(const char *name)
{
	if (can_do_restart(name) < 0) {
		fprintf(stderr, "%s: failed to restart\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else {
		fprintf(stdout, "%s restarted\n", name);
	}
}

static void cmd_restart(const char *name)
{
	do_restart(name);
}

static void do_start(const char *name)
{
	if (can_do_start(name) < 0) {
		fprintf(stderr, "%s: failed to start\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else {
		do_show_state(name);
	}
}

static void cmd_start(const char *name)
{
	do_start(name);
}

static void do_stop(const char *name)
{
	if (can_do_stop(name) < 0) {
		fprintf(stderr, "%s: failed to stop\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else {
		do_show_state(name);
	}
}

static void cmd_stop(const char *name)
{
	do_stop(name);
}

static inline void print_ctrlmode(__u32 cm_flags)
{
	fprintf(stdout,
			"loopback[%s], listen-only[%s], tripple-sampling[%s],"
			"one-shot[%s], berr-reporting[%s]\n",
			(cm_flags & CAN_CTRLMODE_LOOPBACK) ? "ON" : "OFF",
			(cm_flags & CAN_CTRLMODE_LISTENONLY) ? "ON" : "OFF",
			(cm_flags & CAN_CTRLMODE_3_SAMPLES) ? "ON" : "OFF",
			(cm_flags & CAN_CTRLMODE_ONE_SHOT) ? "ON" : "OFF",
			(cm_flags & CAN_CTRLMODE_BERR_REPORTING) ? "ON" : "OFF");
}

static void do_show_ctrlmode(const char *name)
{
	struct can_ctrlmode cm;

	if (can_get_ctrlmode(name, &cm) < 0) {
		fprintf(stderr, "%s: failed to get controlmode\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else {
		fprintf(stdout, "%s ctrlmode: ", name);
		print_ctrlmode(cm.flags);
	}
}

/* this is shamelessly stolen from iproute and slightly modified */
static inline void set_ctrlmode(char* name, char *arg,
		struct can_ctrlmode *cm, __u32 flags)
{
	if (strcmp(arg, "on") == 0) {
		cm->flags |= flags;
	} else if (strcmp(arg, "off") != 0) {
		fprintf(stderr,
				"Error: argument of \"%s\" must be \"on\" or \"off\" %s\n",
				name, arg);
		//exit(EXIT_FAILURE);
		return;
	}
	cm->mask |= flags;
}

static void do_set_ctrlmode(int argc, char* argv[], const char *name)
{
	struct can_ctrlmode cm;

	memset(&cm, 0, sizeof(cm));

	while (argc > 0) {
		if (!strcmp(*argv, "loopback")) {
			NEXT_ARG();
			set_ctrlmode("loopback", *argv, &cm,
					CAN_CTRLMODE_LOOPBACK);
		} else if (!strcmp(*argv, "listen-only")) {
			NEXT_ARG();
			set_ctrlmode("listen-only", *argv, &cm,
					CAN_CTRLMODE_LISTENONLY);
		} else if (!strcmp(*argv, "triple-sampling")) {
			NEXT_ARG();
			set_ctrlmode("triple-sampling", *argv, &cm,
					CAN_CTRLMODE_3_SAMPLES);
		} else if (!strcmp(*argv, "one-shot")) {
			NEXT_ARG();
			set_ctrlmode("one-shot", *argv, &cm,
					CAN_CTRLMODE_ONE_SHOT);
		} else if (!strcmp(*argv, "berr-reporting")) {
			NEXT_ARG();
			set_ctrlmode("berr-reporting", *argv, &cm,
					CAN_CTRLMODE_BERR_REPORTING);
		}

		argc--, argv++;
	}

	if (can_set_ctrlmode(name, &cm) < 0) {
		fprintf(stderr, "%s: failed to set ctrlmode\n", name);
		//exit(EXIT_FAILURE);
		return;
	}
}

static void cmd_ctrlmode(int argc, char *argv[], const char *name)
{
	int show_only = 1;

	if (argc > 0)
		show_only = find_str(config_keywords,
				sizeof(config_keywords) / sizeof(char*),
				argv[1]);

	if (! show_only)
		do_set_ctrlmode(argc, argv, name);

	do_show_ctrlmode(name);
}

static void do_show_restart_ms(const char *name)
{
	__u32 restart_ms;

	if (can_get_restart_ms(name, &restart_ms) < 0) {
		fprintf(stderr, "%s: failed to get restart_ms\n", name);
		//exit(EXIT_FAILURE);
		return;
	} else
		fprintf(stdout,
				"%s restart-ms: %u\n", name, restart_ms);
}

static void do_set_restart_ms(int argc, char* argv[], const char *name)
{
	if (can_set_restart_ms(name,
				(__u32)strtoul(argv[1], NULL, 10)) < 0) {
		fprintf(stderr, "failed to set restart_ms of %s to %lu\n",
				name, strtoul(argv[1], NULL, 10));
		//exit(EXIT_FAILURE);
		return;
	}
}

static void cmd_restart_ms(int argc, char *argv[], const char *name)
{
	int show_only = 1;

	if (argc > 0)
		show_only = find_str(config_keywords,
				sizeof(config_keywords) / sizeof(char*),
				argv[1]);

	if (! show_only)
		do_set_restart_ms(argc, argv, name);

	do_show_restart_ms(name);
}

static void do_show_berr_counter(const char *name)
{
	struct can_berr_counter bc;
	struct can_ctrlmode cm;

	if (can_get_ctrlmode(name, &cm) < 0) {
		fprintf(stderr, "%s: failed to get controlmode\n", name);
		//exit(EXIT_FAILURE);
		return;
	}

	if (cm.flags & CAN_CTRLMODE_BERR_REPORTING) {
		memset(&bc, 0, sizeof(struct can_berr_counter));

		if (can_get_berr_counter(name, &bc) < 0) {
			fprintf(stderr, "%s: failed to get berr counters\n",
					name);
			//exit(EXIT_FAILURE);
			return;
		}

		fprintf(stdout, "%s txerr: %u rxerr: %u\n",
				name, bc.txerr, bc.rxerr);
	}
}

static void cmd_berr_counter(const char *name)
{
	do_show_berr_counter(name);
}

static void cmd_baudrate(const char *name)
{
	fprintf(stderr, "%s: baudrate is deprecated, pleae use bitrate\n",
			name);

	//exit(EXIT_FAILURE);
	return;
}

static void cmd_show_interface(const char *name)
{
	do_show_bitrate(name);
	do_show_bittiming(name);
	do_show_state(name);
	do_show_restart_ms(name);
	do_show_ctrlmode(name);
	do_show_clockfreq(name);
	do_show_bittiming_const(name);
	do_show_berr_counter(name);

	//exit(EXIT_SUCCESS);
	return;
}

//
//can_config - config can
//@argc: similar to main argc
//@argv: similar to main argv
//always return 0
//
//int main(int argc, char *argv[])
int can_config(int argc, char *argv[])
{
	const char* name = argv[1];

	if ((argc < 2) || !strcmp(argv[1], "--help"))
		help();

	if (!strcmp(argv[1], "--version")) {
		printf("Version: %s\n", VERSION);
		//exit(EXIT_SUCCESS);
		return 0;
	}

	if (argc < 3)
		cmd_show_interface(name);

	while (argc-- > 0) {
		if (!strcmp(argv[0], "baudrate"))
			cmd_baudrate( name);
		if (!strcmp(argv[0], "bitrate"))
			cmd_bitrate(argc, argv, name);
		if (!strcmp(argv[0], "bittiming"))
			cmd_bittiming(argc, argv, name);
		if (!strcmp(argv[0], "ctrlmode"))
			cmd_ctrlmode(argc, argv, name);
		if (!strcmp(argv[0], "restart"))
			cmd_restart( name);
		if (!strcmp(argv[0], "start"))
			cmd_start(name);
		if (!strcmp(argv[0], "stop"))
			cmd_stop(name);
		if (!strcmp(argv[0], "restart-ms"))
			cmd_restart_ms(argc, argv, name);
		if (!strcmp(argv[0], "state"))
			cmd_state(name);
		if (!strcmp(argv[0], "clockfreq"))
			cmd_clockfreq(name);
		if (!strcmp(argv[0], "bittiming-constants"))
			cmd_bittiming_const(name);
		if (!strcmp(argv[0], "berr-counter"))
			cmd_berr_counter( name);
		argv++;
	}

	//exit(EXIT_SUCCESS);
	return 0;
}

//
//can_start - start can
//@can_index: can index
//@can_bitrate: can bitrate
//if successed return 0, else return 1
//
int can_start(int can_index, char *can_bitrate)
{
	int can_argc = 4;
	char *can_argv[] = {"canconfig", "can0", "bitrate", "1000000", NULL};

	//parameter check
	if ((can_index != 0) && (can_index != 1))
	{
		APP_ERROR("CAN INDEX NEEDS 0 OR 1.\n");
		return 1;
	}

	if(1 == can_index)
		can_argv[1] = "can1";

	if (can_bitrate != NULL)
		can_argv[3] = can_bitrate;

	//format: canconfig canx bitrate xxxxxxx
	APP_INFOR("can_start: %s %s %s %s\n",
			can_argv[0], can_argv[1], can_argv[2], can_argv[3]);
	can_config(can_argc, can_argv);

	//format: canconfig canx start
	can_argc = 3;
	can_argv[2] = "start";
	can_argv[3] = NULL;
	APP_INFOR("can_start: %s %s %s\n",
			can_argv[0], can_argv[1], can_argv[2]);
	can_config(can_argc, can_argv);

	return 0;
}

//
//can_stop - stop can
//@can_index: can index
//if successed return 0, else return 1
//
int can_stop(int can_index)
{
	int can_argc = 3;
	char *can_argv[] = {"canconfig", "can0", "stop", NULL};

	//parameter check
	if ((can_index != 0) && (can_index != 1))
	{
		APP_ERROR("CAN INDEX NEEDS 0 OR 1.\n");
		return 1;
	}

	if(1 == can_index)
		can_argv[1] = "can1";

	//format: canconfig canx stop
	APP_INFOR("can_stop: %s %s %s\n",
			can_argv[0], can_argv[1], can_argv[2]);
	can_config(can_argc, can_argv);

	return 0;
}

