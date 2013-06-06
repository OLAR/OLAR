#if ANDROID
#include "OLARActivity.h"
#include "../ARSub/ARAndroidCamera.h"
#include "../OLARPlugin.h"

extern "C"{
	
	JNIEXPORT void JNICALL Java_com_olar_OLARPlugin_OLARActivity_nativeOnCreate
	(JNIEnv *env, jobject jthis){
		OLARDUMP("NativeOnCreate");
		JavaVM *vm=NULL;
		if(env->GetJavaVM(&vm)!=JNI_OK){
			OLARDUMP("ARPlugin init : GetJavaVM() failed");
		}
		setJavaVM(vm);
		OLARPlugin *plugin = OLARPlugin::SharedInstance();
		plugin->Initialize();
		plugin->SetPluginActivity(jthis);
	}
	
	JNIEXPORT void JNICALL Java_com_olar_OLARPlugin_OLARActivity_nativeOnDestroy
	(JNIEnv *env, jobject jthis){
		OLARDUMP("NativeOnDestroy");
		OLARPlugin::SharedInstance()->SetPluginActivity(NULL);
	}
	
	JNIEXPORT void JNICALL Java_com_olar_OLARPlugin_OLARActivity_nativeOnResume
	(JNIEnv *env, jobject jthis){
		OLARPlugin *plugin = OLARPlugin::SharedInstance();
		bool started = plugin->IsStarted();
		bool running = plugin->IsRunning();
		OLARDUMP("NativeOnResume started = %d,running = %d",started,running);
		if(started){
			plugin->SetNeedRestart(true);
		}
	}
	
	JNIEXPORT void JNICALL Java_com_olar_OLARPlugin_OLARActivity_nativeOnPause
	(JNIEnv *env, jobject jthis){
		OLARDUMP("NativeOnPause");
	}
	
	JNIEXPORT void JNICALL Java_com_olar_OLARPlugin_OLARActivity_nativeOnCaptureFrame
	(JNIEnv *env, jobject jthis, jlong cameraAddr, jbyteArray data){
		((ARAndroidCamera*)cameraAddr)->OnCaptureFrame(data);
	}
	
	JNIEXPORT void JNICALL Java_com_olar_OLARPlugin_OLARActivity_nativeSetCameraConfig
	(JNIEnv *env, jobject jthis, jint width, jint height, jint pixelFormat, jdouble fov){
		OLARDUMP("NativeSetCameraConfig %dx%d format=%x fov=%f",width,height,pixelFormat,fov);
		OLARPlugin::SharedInstance()->SetCameraConfig(width,height,pixelFormat,fov);
	}
	
}

#endif
