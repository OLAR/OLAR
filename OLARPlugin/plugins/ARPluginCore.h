#ifndef __OLAR_PLUGIN_CORE_H__
#define __OLAR_PLUGIN_CORE_H__

#include <stdint.h>
#include "cocoa/CCObject.h"

struct OLARResult{
	int   targetId;
	int   resultId;
	int   count;
	float error;
	float confidence;
	float modelViewMatrix[16];
	OLARResult *nextResult;
};

class OLARTarget : public cocos2d::CCObject{
	friend class OLAREngine;
	struct OLARTargetData *_data;
protected:
	void   Reset();
	void   UpdateResults(OLARResult *result, int count);
public:
	static OLARTarget* TargetWithPath(const char *path);
	static OLARTarget* TargetWithData(void *data,const int length);
	OLARTarget();
	~OLARTarget();
	bool   LoadData(void *data, const int length);
	void   SetSize(float size);
	void   SetFilterLevel(float filterLevel);
	float  Width();
	float  Height();
	void*  Engine();
	void   SetEngine(void *engine);
	bool   IsSizeSpecified();
	int    TargetId();
	void*  TargetData();
	OLARResult* LastResult();
};

class OLAREngine{
	struct OLAREngineData *_data;
public:
	void Initialize(int w, int h, int fmt);
	void Release();
	int  Track(void *data);
	void AddTarget(OLARTarget *target);
	void RemoveTarget(OLARTarget *target);
	void RemoveAllTargets();
	int  NumberOfTargets();
	int  EngineType();
	OLARResult* LastResult();
	OLARTarget* DefaultTarget();
	float* ProjectionMatrix(const float farClip);
	void GetCameraMatrix(float cparam[3][4]);
	static void SetLoadBytesCallback(void*(*callback)(const char*,int*));
private:
	void UpdateTargetResults();
};

#endif
