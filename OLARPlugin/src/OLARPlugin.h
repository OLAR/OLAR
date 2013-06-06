#ifndef __OLAR_PLUGIN_H__
#define __OLAR_PLUGIN_H__

#if ANDROID
#include <jni.h>
#endif

#include <cocos2d.h>
#include "ARPluginCore.h"

class OLARTarget;
typedef struct OLARResult OLARResult;

struct OLARImage{
	int width;
	int height;
	int bpp;
	uint8_t *data;
	uint8_t *extdata;
};

class OLARPlugin{
	struct OLARPluginData *_data;
public:
	static OLARPlugin* SharedInstance();
	
	OLARPlugin();
	~OLARPlugin();
	void Initialize();
	void Release();
	bool Start();
	bool Stop();
	OLARResult* LastResult();
	void AddTarget(OLARTarget *target);
	void RemoveTarget(OLARTarget *target);
	void RemoveAllTargets();
	void RenderCameraImage();
	bool GetCameraImage(OLARImage *image);
	bool IsInitialized();
	bool IsRunning();
	bool IsStarted();
	void GetProjectionMatrix(float *proj,float farClip);
	int  GetDeviceOrientation();
	
	// cocos2d related
	cocos2d::CCPoint ConvertCocosPosToCameraPos(cocos2d::CCPoint p);
	cocos2d::CCPoint GetLocationInTarget(float x0, float y0);
	
	// For iOS simulator
	static bool IsSimulator();
	static void SetSimulatorData(const char *path,int previewFPS=30);
	
public:
	// internal use
	void      SetCameraConfig(int width,int height,int pixelFormat,float fov);
	bool      IsNeedToRestart();
	void      SetNeedRestart(bool v);
	void      CaptureCallback(void*);
	uint8_t*  LoadDataFromFile(const char *path,int *length);
	void      UpdateLastMVP(OLARResult *result);
	
	// For android
	void      SetPluginActivity(void* activity);
	void*     GetPluginActivity();
};

class OLARArray;
class OLARSceneEventListener;

class OLARScene : public cocos2d::CCScene{
protected:
	OLARTarget  *detectedTarget_;
	OLARArray *targets_;
	float     transform_[16];
	OLARArray *listeners_;
	int       frameCount_;
public:
	OLARScene();
	virtual ~OLARScene();
	virtual bool init();
	virtual void visit();
	virtual void onEnter();
	virtual void onEnterTransitionDidFinish();
    virtual void onExit();
	OLARTarget* TargetAtIndex(int index);
	OLARTarget* DetectedTarget();
	void AddTarget(OLARTarget *target);
	void RemoveTarget(OLARTarget *target);
	int  NumberOfTargets();
	void AddEventListener(OLARSceneEventListener *listener);
	void RemoveEventListener(OLARSceneEventListener *listener);
	cocos2d::CCPoint convertToNodeSpace(const cocos2d::CCPoint& worldPoint);
	static OLARScene* create();
	static OLARScene* createWithTarget(OLARTarget *target);
};

class OLARSceneEventListener : public cocos2d::CCObject{
public:
	virtual void OnVisible(OLARScene* scene){}
	virtual void OnInvisible(OLARScene* scene){}
	virtual void OnUpdate(OLARScene* scene){}
};

class OLARTouchDelegate : public cocos2d::CCTouchDelegate, public cocos2d::CCObject{
public:
	OLARTouchDelegate();
	virtual bool ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
    virtual void ccTouchMoved(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
    virtual void ccTouchEnded(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
    virtual void ccTouchCancelled(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
	void ConvertTouches(cocos2d::CCTouch *touch);
};

#ifndef OLARDUMP
#if ANDROID
#include <android/log.h>
#define OLARDUMP(...)   __android_log_print(ANDROID_LOG_INFO,"OLAR",__VA_ARGS__)
#else
#define OLARDUMP(...) { fprintf(stderr, "[OLAR]" __VA_ARGS__); fputc('\n',stderr); }
#endif
#endif

#endif
