/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class jp_qoncept_ARPlugin_ARPluginActivity */

#ifndef _Included_jp_qoncept_ARPlugin_ARPluginActivity
#define _Included_jp_qoncept_ARPlugin_ARPluginActivity
#ifdef __cplusplus
extern "C" {
#endif
#undef jp_qoncept_ARPlugin_ARPluginActivity_HANDLER_SHOW_DIALOG
#define jp_qoncept_ARPlugin_ARPluginActivity_HANDLER_SHOW_DIALOG 1L
#undef jp_qoncept_ARPlugin_ARPluginActivity_MP
#define jp_qoncept_ARPlugin_ARPluginActivity_MP -1L
#undef jp_qoncept_ARPlugin_ARPluginActivity_QCDeviceOrientationUnknown
#define jp_qoncept_ARPlugin_ARPluginActivity_QCDeviceOrientationUnknown 0L
#undef jp_qoncept_ARPlugin_ARPluginActivity_QCDeviceOrientationPortrate
#define jp_qoncept_ARPlugin_ARPluginActivity_QCDeviceOrientationPortrate 1L
#undef jp_qoncept_ARPlugin_ARPluginActivity_QCDeviceOrientationLandscapeLeft
#define jp_qoncept_ARPlugin_ARPluginActivity_QCDeviceOrientationLandscapeLeft 3L
#undef jp_qoncept_ARPlugin_ARPluginActivity_ARCameraFormatBGRA
#define jp_qoncept_ARPlugin_ARPluginActivity_ARCameraFormatBGRA 65536L
#undef jp_qoncept_ARPlugin_ARPluginActivity_ARCameraFormatYUVSP
#define jp_qoncept_ARPlugin_ARPluginActivity_ARCameraFormatYUVSP 131072L
/*
 * Class:     jp_qoncept_ARPlugin_ARPluginActivity
 * Method:    nativeOnCreate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_jp_qoncept_ARPlugin_ARPluginActivity_nativeOnCreate
  (JNIEnv *, jobject);

/*
 * Class:     jp_qoncept_ARPlugin_ARPluginActivity
 * Method:    nativeOnDestroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_jp_qoncept_ARPlugin_ARPluginActivity_nativeOnDestroy
  (JNIEnv *, jobject);

/*
 * Class:     jp_qoncept_ARPlugin_ARPluginActivity
 * Method:    nativeOnResume
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_jp_qoncept_ARPlugin_ARPluginActivity_nativeOnResume
  (JNIEnv *, jobject);

/*
 * Class:     jp_qoncept_ARPlugin_ARPluginActivity
 * Method:    nativeOnPause
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_jp_qoncept_ARPlugin_ARPluginActivity_nativeOnPause
  (JNIEnv *, jobject);

/*
 * Class:     jp_qoncept_ARPlugin_ARPluginActivity
 * Method:    nativeOnCaptureFrame
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_jp_qoncept_ARPlugin_ARPluginActivity_nativeOnCaptureFrame
  (JNIEnv *, jobject, jlong, jbyteArray);

/*
 * Class:     jp_qoncept_ARPlugin_ARPluginActivity
 * Method:    nativeSetCameraConfig
 * Signature: (IIID)V
 */
JNIEXPORT void JNICALL Java_jp_qoncept_ARPlugin_ARPluginActivity_nativeSetCameraConfig
  (JNIEnv *, jobject, jint, jint, jint, jdouble);

#ifdef __cplusplus
}
#endif
#endif
