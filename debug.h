//
//Code for LSCM - debug.h
//Copyright (C) 2013 SIBET. - http://www.sibet.ac.cn/
//Created by Gao Fei in February 5, 2013
//

#ifndef	DEBUG_H
#define	DEBUG_H

#undef	APP_INFOR
#undef	APP_ERROR

#ifdef	LSCM_DEBUG
#define	APP_INFOR(fmt, args...)	printf(fmt, ##args)
#define	APP_ERROR(fmt, args...)	printf("ERROR: "fmt, ##args)
#else
#define	APP_INFOR(fmt, args...)
#define	APP_ERROR(fmt, args...)
#endif

#endif

