#ifndef __OLAR_AR_COMMON_H__
#define __OLAR_AR_COMMON_H__

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if ANDROID
#include <jni.h>
#include <platform/android/jni/JniHelper.h>
#define TARGET_IPHONE_SIMULATOR 0
#define YES (1)
#define NO  (0)
#endif

#if __OBJC__
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#endif

#define AR_MAX_RESULTS (64)
#define AR_MAX_TARGETS (64)
#define AR_ALLOC(T,...) (new T(__VA_ARGS__))
#define AR_RELEASE(p,T) if(p){ delete (T*)(p); (p)=NULL; }
#define AR_ALLOC_ARRAY(T,n) (new T[n])
#define AR_RELEASE_ARRAY(p,T) if( p ){ delete [] (p); (p)=NULL; }

#if ANDROID
#include <android/log.h>
#define OLARDUMP(...)   __android_log_print(ANDROID_LOG_INFO,"OLAR",__VA_ARGS__)
#else
#define OLARDUMP(...) { fprintf(stderr, "[OLAR]" __VA_ARGS__); fputc('\n',stderr); }
#endif

#if ANDROID
void setJavaVM(JavaVM *vm);
JavaVM *getJavaVM();
JNIEnv *getJNIEnv();
JNIEnv *getJNIEnvX(int *attached);
#else
typedef void* jobject;
#endif

class QCLock{
	pthread_mutex_t lock;
public:
	QCLock(){
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&lock,&attr);
	}
	~QCLock(){
		pthread_mutex_destroy(&lock);
	}
	void Lock(){ pthread_mutex_lock(&lock); }
	void Unlock(){ pthread_mutex_unlock(&lock); }
};

class QCScopedLock{
	QCLock *lock;
public:
	QCScopedLock(QCLock *_lock){ (lock=_lock)->Lock(); }
	~QCScopedLock(){ lock->Unlock(); }
};

typedef enum{
	AROrientationUnknown = 0,
	AROrientationPortrait = 1,
	AROrientationPortraitUpsideDown = 2,
	AROrientationLandscapeLeft = 4,
	AROrientationLandscapeRight = 3,
}AROrientation;

#if ANDROID
int      ARGetDeviceOrientation(jobject activity);
uint8_t* ARLoadDataFromFile(jobject activity,const char *path,int *length);
#else
int      ARGetDeviceOrientation();
uint8_t* ARLoadDataFromFile(const char *path,int *length);
#endif

class QMPoint{
private:
	float m_X;
	float m_Y;
public:
	QMPoint();
	QMPoint(float x,float y);
	float x() const;
	void setX(float x);
	float y() const;
	void setY(float y);
	
	char *cstr() const;
};

class QMSize{
private:
	float m_Width;
	float m_Height;
public:
	QMSize();
	QMSize(float w,float h);
	float width() const;
	float height() const;
	
	char *cstr() const;
};

class QMRect{
private:
	float m_Left;
	float m_Top;
	float m_Width;
	float m_Height;
public:
	QMRect();
	QMRect(float l,float t,float w,float h);
	float left() const;
	float top() const;
	float right() const;
	float bottom() const;
	float width() const;
	float height() const;
	QMPoint center() const;
	QMSize size() const;
	
	QMRect aspectWidth(const QMSize &size) const;
	QMRect aspectHeight(const QMSize &size) const;
	QMRect aspectFit(const QMSize &size) const;
	
	char *cstr() const;
};

char *CStrFromMatrix4x4(float *m);

#define OLAR_ARRAY_DEFAULT_CAPACITY (64)

class OLARArray{
	int    count;
	int    capacity;
	int    capacity0;
	void **items;
public:
	OLARArray();
	OLARArray(int _count);
	~OLARArray();
	void** Items();
	int    Count();
	bool   Add(void *item);
	bool   Remove(int index);
	bool   Remove(void *item);
	void*  ItemAt(int index);
	int    IndexOf(void *item);
	void   Clear();
	void   Release();
	void   Resize(int _count);
	void   Dump();
private:
	void   ResizeCapacity(int _capacity);
};

#endif
