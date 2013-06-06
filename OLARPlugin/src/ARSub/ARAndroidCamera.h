#ifndef __OLAR_AR_ANDROID_CAMERA_H__
#define __OLAR_AR_ANDROID_CAMERA_H__
#if ANDROID

#include "ARCamera.h"
#include <jni.h>

class ARAndroidCamera : public ARCamera{
public:
private:
	jobject jActivity;
	jclass jActivityClass;
	jmethodID jCameraStart;
	jmethodID jCameraStop;
	jmethodID jCameraIsRunning;
	jfieldID jNativeCameraAddr;
public:
	ARAndroidCamera();
	virtual ~ARAndroidCamera();
	void SetPluginActivity(jobject activity);
	virtual bool Start();
	virtual bool Stop();
	virtual bool IsRunning();
	void OnCaptureFrame(jbyteArray data);
};

#endif
#endif
