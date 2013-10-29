#ifndef _NET_PROTOCOL_H
#define _NET_PROTOCOL_H

#define TOKEN_SIZE 34 
// Device

#define HD_LASER            0x00    // 405nm, 488nm, 543nm, 638nm四路激光器
#define HD_AOTF             0x01    // AOTF硬件
#define HD_SLIT				0x02    //狭缝电机用于光谱成像的狭缝 
#define HD_DICHROIC			0x03	//二色镜电机	
#define HD_ARB				0x04	//振镜控制及采样控制板
#define HD_PMTGAIN         0x08    // 三路PMT的增益
#define HD_OBJECTIVELENS   0x09   // 物镜10X, 40X, 60X油, 100X油
#define HD_XYSTAGE         0x0a    // XY载物台
#define HD_PINHOLE          0x0e    // 三个针孔

// Command
#define CMD_SET            0x00    // 控制硬件设备命令
#define CMD_GET            0x01    // 获取硬件设备状态命令
#define STATUS_SET_OK      0x02    // 设置状态ok
#define STATUS_GET_OK      0x03    // 获取硬件设备状态ok
#define STATUS_SET_ERR	   0x04	   //设置状态错误
#define STATUS_GET_ERR	   0x05	    //获取状态错误

typedef struct {
	unsigned char startSign;
	unsigned char Device;
	unsigned char Command;
	unsigned char Val[30];
	unsigned char tokenEnd;
}netToken;
#endif 
