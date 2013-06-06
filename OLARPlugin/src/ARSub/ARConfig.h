#ifndef __OLAR_AR_CONFIG_H__
#define __OLAR_AR_CONFIG_H__

#define ARCameraSizeCustom    0x00000000
#define ARCameraSize480x360   0x00000100
#define ARCameraSize640x480   0x00000200

#define ARCameraFrameRate00   0
#define ARCameraFrameRate01   1
#define ARCameraFrameRate05   5
#define ARCameraFrameRate15   15
#define ARCameraFrameRate24   24
#define ARCameraFrameRate30   30
#define ARCameraFrameRate60   60

#define ARCameraFormatBGRA    0x00010000
#define ARCameraFormatYUVSP   0x00020000

#define ARSimulatorMode       0X01000000

typedef struct ARConfigT{
	bool simulatorMode;
	bool autoStart;
	int  sizePreset;
	int  frameRate;
	int  pixelFormat;
}ARConfig;

extern ARConfig ARDefaultConfig;

#ifndef __AR_IMAGE_DEFINED__
#define __AR_IMAGE_DEFINED__
typedef struct ARImageT{
	int width;
	int height;
	int bpp;
	int pitch;
	unsigned char *data;
	int pixelFormat;
	int bufferSize;
	int bufferNum;
}ARImage;
#endif

#endif
