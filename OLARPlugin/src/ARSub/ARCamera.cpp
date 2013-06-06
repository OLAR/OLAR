#include "ARCommon.h"
#include "ARCamera.h"
#include "ARGLUtil.h"

static ARCamera *__defaultCamera = NULL;

#if !ANDROID
bool QCCIsMainThread(){
	return [NSThread isMainThread]? true : false;
}
#endif

ARConfig ARDefaultConfig = { 
	TARGET_IPHONE_SIMULATOR, 
	YES,
	ARCameraSize640x480, 
	ARCameraFrameRate30, 
	ARCameraFormatBGRA
};

ARCamera* ARCamera::GetDefaultCamera(bool useSimulator){
	ARCamera *camera = NULL;
	if( __defaultCamera ){
		return __defaultCamera;
	}
#if ANDROID
	camera = AR_ALLOC(ARAndroidCamera);
#elif !TARGET_IPHONE_SIMULATOR
	if( useSimulator ){
		camera = AR_ALLOC(ARSimulatorCamera);
	}else{
		camera = AR_ALLOC(ARDeviceCamera);
	}
#else
	camera = AR_ALLOC(ARSimulatorCamera);
#endif
	__defaultCamera = camera;
	return camera;
}

void ARCamera::ReleseDefaultCamera(){
	AR_RELEASE(__defaultCamera, ARCamera);
}


ARCamera::ARCamera(){
	callback = NULL;
	callbackArg = NULL;
	captureImage = AR_ALLOC(ARImage);
	captureImage->width = 0;
	captureImage->height = 0;
	captureImage->bpp = 0;
	captureImage->pitch = 0;
	captureImage->data = NULL;
	buffers[0] = NULL;
	buffers[1] = NULL;
	activeIndex = 1;
	mainTextureId = 0;
	uvTextureId = 0;
	pthread_mutex_init(&bufferMutex, NULL);
}

void ARCamera::SetCaptureCallback(ARCameraCallback _callback, void *args){
	callback = _callback;
	callbackArg = args;
}

bool ARCamera::Start(){ return false; }
bool ARCamera::Stop(){ return false; }
bool ARCamera::Pause(){ return false; }
void ARCamera::SetFrameRate(float fps){
	frameRate = fps;
}


bool ARCamera::IsSimulator(){
	return true;
}

bool ARCamera::SetSimulatorData(const char *path){
	return false;
}

int ARCamera::Width(){
	return captureImage->width;
}

int ARCamera::Height(){
	return captureImage->height;
}

int ARCamera::Bpp(){
	return captureImage->bpp;
}

void* ARCamera::PixelBuffer(){
	return (void*)buffers[activeIndex];
}

void* ARCamera::PixelBuffer(int plane){
	unsigned char *p = buffers[activeIndex];
	int offset = (captureImage->pitch*captureImage->height)*plane;
	return (void*)(p+offset);
}

bool ARCamera::IsRunning(){
	return NO;
}

void ARCamera::LockBuffer(){
	pthread_mutex_lock(&bufferMutex);
}

void ARCamera::UnlockBuffer(){
	pthread_mutex_unlock(&bufferMutex);
}


void ARCamera::SetTextures(uint32_t mainTex, uint32_t uvTex ){
	mainTextureId = mainTex;
	uvTextureId = uvTex;
	if( mainTextureId == 0 ){ return; }
	int w = captureImage->width;
	int h = captureImage->height;
	if( captureImage->bpp == 1 ){
		glBindTexture(GL_TEXTURE_2D,mainTextureId);
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,w,h,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,NULL);
	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		if( uvTextureId ){
			glBindTexture(GL_TEXTURE_2D,uvTextureId);
			glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE_ALPHA,w/2,h/2,0,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,NULL);

			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			
		}
	}else if( captureImage->bpp == 4 ){
		glBindTexture(GL_TEXTURE_2D,mainTextureId);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_BGRA_EXT,GL_UNSIGNED_BYTE,NULL);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void ARCamera::UpdateTexture(){
	LockBuffer();
	unsigned char *yplane = buffers[1-activeIndex];
	unsigned char *uvplane = yplane + (captureImage->pitch*captureImage->height);
	int w = captureImage->width;
	int h = captureImage->height;
	if( captureImage->bpp == 1 ){
		if( mainTextureId ){
			glBindTexture(GL_TEXTURE_2D,mainTextureId);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_LUMINANCE, GL_UNSIGNED_BYTE, yplane);
		}
		if( uvTextureId ){
			glBindTexture(GL_TEXTURE_2D,uvTextureId);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w/2, h/2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, uvplane);
		}
	}else if( captureImage->bpp == 4 ){
		if( mainTextureId ){
			glBindTexture(GL_TEXTURE_2D,mainTextureId);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, yplane);
		}
	}
	glBindTexture(GL_TEXTURE_2D,0);
	UnlockBuffer();
}

ARConfig ARCamera::CurrentConfig(){
	return currentConfig;
}

bool ARCamera::InitCaptureImage(int width,int height,int pixelFormat,int bufferNum){
	if( captureImage->data ){
		AR_RELEASE_ARRAY(captureImage->data,uint8_t);
		captureImage->data = NULL;
	}
	captureImage->width = width;
	captureImage->height = height;
	captureImage->pixelFormat = pixelFormat;
	captureImage->bufferNum = bufferNum;
	
	captureImage->bufferSize = 0;
	if( pixelFormat == ARCameraFormatYUVSP ){
		captureImage->bpp = 1;
		captureImage->pitch = captureImage->width*captureImage->bpp;
		captureImage->bufferSize = captureImage->width*captureImage->height*2;
	}else {
		captureImage->bpp = 4;
		captureImage->pitch = captureImage->width*captureImage->bpp;
		captureImage->bufferSize = captureImage->width*captureImage->height*4;
	}
	captureImage->data = AR_ALLOC_ARRAY(uint8_t,captureImage->bufferSize*captureImage->bufferNum);
	if( captureImage->pixelFormat == ARCameraFormatYUVSP ){
		for(int i=0;i<captureImage->bufferNum;i++){
			unsigned char *yplane = captureImage->data + captureImage->bufferSize * i;
			unsigned char *uvplane = yplane + (captureImage->pitch*captureImage->height);
			memset(yplane,0,captureImage->pitch*captureImage->height);
			memset(uvplane,128,captureImage->pitch*captureImage->height/2);
		}
	}else{
		memset(captureImage->data,0,captureImage->bufferSize*captureImage->bufferNum);
	}
	return true;
}

bool ARCamera::SetCaptureImageConfig(int width,int height,int pixelFormat){
	if(!InitCaptureImage(width,height,pixelFormat,2))return false;
	buffers[0] = captureImage->data;
	buffers[1] = captureImage->data + captureImage->bufferSize;
	activeIndex = 1;
	OLARDUMP("ARCamera::SetCaptureImageConfig : %dx%d format=%x size=%d buf0=%p,buf1=%p",
		 width,height,pixelFormat,captureImage->bufferSize,buffers[0],buffers[1]);
	return true;
}


bool ARCamera::SetConfig(ARConfig *config){
	currentConfig = *config;
	
	frameRate = config->frameRate;
	if( frameRate < 0 ){ frameRate = 0; }
	
	int w,h;
	if( config->sizePreset == ARCameraSize640x480 ){
		w  = 640;
		h = 480;
	}else {
		w  = 480;
		h = 360;
	}
	if(!SetCaptureImageConfig(w,h,config->pixelFormat))return false;

	return true;
}

ARCamera::~ARCamera(){
	if( captureImage ){
		if( captureImage->data ){
			AR_RELEASE_ARRAY(captureImage->data,uint8_t);
		}
		AR_RELEASE(captureImage,ARImage);
	}
}
