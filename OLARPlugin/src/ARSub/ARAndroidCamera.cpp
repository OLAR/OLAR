#if ANDROID
#include "ARAndroidCamera.h"
#include "ARCommon.h"
#include <string.h>

ARAndroidCamera::ARAndroidCamera():ARCamera(),jActivity(NULL){
	JNIEnv *env = getJNIEnv();
	
	if( env == NULL ){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return;
	}
	
	jActivityClass = env->FindClass("com/olar/OLARPlugin/OLARActivity");
	if( jActivityClass == NULL ){
		OLARDUMP("%s:FindClass(OLARActivity) is null",__FUNCTION__);
		return;
	}
	
	jCameraStart=env->GetMethodID(jActivityClass,"cameraStart","()Z");
	if( jCameraStart == NULL ){
		OLARDUMP("%s:GetMethodID(cameraStart) is null",__FUNCTION__);
		return;
	}
	
	jCameraStop=env->GetMethodID(jActivityClass,"cameraStop","()Z");
	if( jCameraStop == NULL ){
		OLARDUMP("%s:GetMethodID(cameraStop) is null",__FUNCTION__);
		return;
	}
	
	jCameraIsRunning = env->GetMethodID(jActivityClass,"cameraIsRunning","()Z");
	if( jCameraStart == NULL ){
		OLARDUMP("%s:GetMethodID(cameraIsRunning) is null",__FUNCTION__);
		return;
	}
	
	jNativeCameraAddr = env->GetFieldID(jActivityClass,"nativeCameraAddr","J");
	if( jNativeCameraAddr == NULL ){
		OLARDUMP("%s:GetFieldID(nativeCameraAddr) is null",__FUNCTION__);
		return;
	}
}

ARAndroidCamera::~ARAndroidCamera(){
	SetPluginActivity(NULL);
}

void ARAndroidCamera::SetPluginActivity(jobject activity){
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return;
	}
	if(jActivity!=NULL){
		env->SetLongField(jActivity,jNativeCameraAddr,static_cast<jlong>(0));
		env->DeleteGlobalRef(jActivity);
		jActivity=NULL;
	}
	if(activity!=NULL){
		jlong addr = reinterpret_cast<jlong>(this);
		OLARDUMP("cameraAddr=%p",this);
		jActivity=env->NewGlobalRef(activity);
		env->SetLongField(jActivity,jNativeCameraAddr,addr);
	}
}

bool ARAndroidCamera::Start(){
	OLARDUMP("native ARAndroidCamera Start");
	if(jActivity==NULL)return false;
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return false;
	}
	return env->CallBooleanMethod(jActivity,jCameraStart);
}
bool ARAndroidCamera::Stop(){
	OLARDUMP("native ARAndroidCamera Stop");
	if(jActivity==NULL)return false;
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return false;
	}
	return env->CallBooleanMethod(jActivity,jCameraStop);
}
bool ARAndroidCamera::IsRunning(){
	//OLARDUMP("native ARAndroidCamera IsRunning");
	if(jActivity==NULL)return false;
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return false;
	}
	return env->CallBooleanMethod(jActivity,jCameraIsRunning);
}

void ARAndroidCamera::OnCaptureFrame(jbyteArray data){
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return;
	}
	jsize dataLen = env->GetArrayLength(data);
	jbyte *dataPtr = env->GetByteArrayElements(data,NULL);
	
	(void)dataLen;
	
	int yplaneSize = captureImage->height * captureImage->pitch;
	int uvplaneSize = yplaneSize/2;

	
	//OLARDUMP("%s:dataLen=%d,yplaneSize=%d,uvPlaneSize=%d,buf0=%p,buf1=%p",
	//	 __PRETTY_FUNCTION__,dataLen,yplaneSize,uvplaneSize,buffers[0],buffers[1]);

	uint8_t *yplane = NULL;
	uint8_t *uvplane = NULL;
	if( captureImage->pixelFormat == ARCameraFormatYUVSP ){
		yplane = reinterpret_cast<uint8_t *>(dataPtr);
		uvplane = yplane + yplaneSize;
		
	}else if(captureImage->pixelFormat == ARCameraFormatBGRA ){
		OLARDUMP("%s:not support BGRA",__FUNCTION__);
	}
	
	LockBuffer();
	unsigned char *dst = buffers[1-activeIndex];
	if(yplane){
		memcpy(dst,yplane,yplaneSize);
	}
	if(uvplane){
		memcpy(dst+yplaneSize,uvplane,uvplaneSize);
	}
	activeIndex = 1 - activeIndex;
	UnlockBuffer();
	
	if(callback)callback(dst,callbackArg);

	env->ReleaseByteArrayElements(data,dataPtr,0);
}

#endif