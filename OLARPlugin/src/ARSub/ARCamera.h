#ifndef __OLAR_AR_CAMERA_H__
#define __OLAR_AR_CAMERA_H__

#include "ARConfig.h"
#include "ARCommon.h"

typedef void (*ARCameraCallback)(void*,void*);

class ARCamera{
public:
protected:
	float             frameRate;
	bool              captured;
	ARCameraCallback  callback;
	void*             callbackArg;
	ARConfig          currentConfig;
	int               activeIndex;
	uint8_t          *buffers[2];
	ARImage          *captureImage;
	uint32_t          mainTextureId;
	uint32_t          uvTextureId;
	pthread_mutex_t   bufferMutex;

	virtual bool InitCaptureImage(int width,int height,int pixelFormat,int bufferNum);

public:
	ARCamera();
	virtual ~ARCamera();
	
	virtual bool SetCaptureImageConfig(int width,int height,int pixelFormat);
	virtual bool SetConfig(ARConfig *config);
	
	
	virtual bool IsRunning();
	virtual bool IsSimulator();
	virtual bool SetSimulatorData(const char *path);
	virtual bool  Start();
	virtual bool  Pause();
	virtual bool  Stop();
	virtual void  SetFrameRate(float fps);
	void  SetCaptureCallback(ARCameraCallback callback, void *args);
	int   Width();
	int   Height();
	int   Bpp();
	void* PixelBuffer();
	void* PixelBuffer(int plane);
	void  UpdateTexture();
	void  SetTextures(uint32_t mainTex,uint32_t uvTex);
	void  LockBuffer();
	void  UnlockBuffer();
	ARConfig CurrentConfig();
public:
	static ARCamera* GetDefaultCamera(bool useSimulator);
	static void ReleseDefaultCamera();
};

class ARDeviceCamera : public ARCamera{
	void *session;
	void *delegate;
public:
	ARDeviceCamera();
	virtual ~ARDeviceCamera();
	virtual bool  Start();
	virtual bool  Pause();
	virtual bool  Stop();
	virtual bool SetConfig(ARConfig *config);
	virtual bool IsRunning();
	virtual bool IsSimulator();
	void OnCaptureFrame(void *buffer);
};

class  ARSimulatorCamera : public ARCamera{
	struct ARSimulatorCameraData *data;
public:
	ARSimulatorCamera();
	virtual ~ARSimulatorCamera();
	virtual bool  Start();
	virtual bool  Pause();
	virtual bool  Stop();
	virtual bool  IsRunning();
	virtual bool  IsSimulator();
	virtual bool  SetSimulatorData(const char *path);
	void OnTimer();
	void OnCaptureFrame(void *buffer);
};

#if ANDROID
#include "ARAndroidCamera.h"
#endif

#endif
