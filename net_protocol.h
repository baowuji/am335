#ifndef _NET_PROTOCOL_H
#define _NET_PROTOCOL_H

#define TOKEN_SIZE 22
// Device
#define HD_READY          0x01    // 开始连接
#define HD_CANCEL         0x02    // 取消连接
#define HD_OK              0x03    // 执行成功，返回指令
#define HD_ERR             0x04    // 执行失败，返回指令

#define HD_AOTF             0x05    // AOTF硬件
#define HD_XYSCANNER      0x06    // XY扫描振镜
#define HD_LASER            0x07    // 405nm, 488nm, 543nm, 638nm四路激光器
#define HD_PMTGAIN         0x08    // 三路PMT的增益
#define HD_OBJECTIVELENS   0x09   // 物镜10X, 40X, 60X油, 100X油
#define HD_XYSTAGE         0x0a    // XY载物台
#define HD_SLIT              0x0b    // 两路用于光谱成像的狭缝
#define HD_SPECTRALSWITCH 0x0c    // 蓝光、红光光谱单元切换控制器
#define HD_DICHROICMIRROR 0x0d    // 二色镜转盘
#define HD_PINHOLE          0x0e    // 三个针孔

// Command
#define FUNC_GET            0x01    // 获取硬件设备状态
#define FUNC_SET            0x02    // 控制硬件设备

// Type
#define LASER405       0x01          // 四路激光器
#define LASER488       0x02
#define LASER543       0x03
#define LASER638       0x04

#define X_VOLTAGERANGE 0x01         // XY扫描振镜相关控制
#define Y_VOLTAGERANGE 0x02
#define X_FREQUENCY    0x03
#define Y_FREQUENCY    0x04
#define XY_FREQUENCY   0x05
#define PIXELCLOCK     0x06

#define PMT1           0x01            // 3路PMT
#define PMT2           0x02
#define PMT3           0x03

#define XSTAGE_RANGE   0x01          // XY载物台相关控制
#define YSTAGE_RANGE   0x02
#define XSTAGE_POS     0x03
#define YSTAGE_POS     0x04
#define XYSTAGE_POS    0x05

#define FIRSTSLIT_L_POS   0x01         // 2路狭缝
#define FIRSTSLIT_R_POS   0x02
#define FIRSTSLIT_LR_POS  0x03
#define SECONDSLIT_L_POS  0x04
#define SECONDSLIT_R_POS  0x05
#define SECONDSLIT_LR_POS 0x06

#define FIRST_SPECTRAL    0x01    // NONE BLUE     // 蓝，绿光谱切换相关控制
#define SECOND_SPECTRAL   0x02    // NONE GREEN


// Status
enum FirstSpectralStatus{FirstSpectral_None = 0, FirstSpectral_Blue = 1};                      // 蓝色光谱切换状态
enum SecondSpectralStatus{SecondSpectral_None = 0, SecondSpectral_Green = 1};              // 绿色光谱切换状态
enum LaserSwitchStatus{OFF = 0, ON = 1};                                               // 激光器开关状态
enum ObjectiveLensStatus{Objective_None = 0, Objective_10X = 1, Objective_40X = 2, Objective_60XOil = 3, Objective_100XOil = 4}; // 物镜选择状态
enum PinholeStatus{Pinhole1 = 0, Pinhole2 = 1, Pinhole3 = 2};                                // 针孔选择状态
enum DichroicMirrorStatus{DM_None = 0, DM_405 = 1, DM_488 = 2, DM_543 = 3, DM_638 = 4};  // 二色镜转盘选择状态

typedef struct {
	unsigned char startSign;
	unsigned char Device;
	unsigned char Command;
	unsigned char Val[30];
	unsigned char tokenEnd;
}netToken;
#endif 
