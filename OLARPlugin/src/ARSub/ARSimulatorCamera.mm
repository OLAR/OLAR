#define AR_CAMERA_PRIVATE
#include "ARCommon.h"
#include "ARCamera.h"
#include "ARDecoder.h"

static char* PathExtension(const char *path);
static bool  CaseInsentiveEqual(const char *str1, const char *str2);
static bool  HasSuffix(const char *str, const char *sfx);

@interface NSTimerDelegate : NSObject{
	@public ARSimulatorCamera *camera; 
}
-(void)onTimer;
@end
@implementation NSTimerDelegate
-(void)onTimer{ if( camera ){ camera->OnTimer(); } }
@end

struct ARSimulatorCameraData {
	NSTimer *timer;
	NSTimerDelegate *delegate;
	ARDecoder *decoder;
};

ARSimulatorCamera::ARSimulatorCamera() : ARCamera(){
	data = AR_ALLOC(ARSimulatorCameraData);
	data->timer = NULL;
	data->decoder = NULL;
}

ARSimulatorCamera::~ARSimulatorCamera(){
	Stop();
	AR_RELEASE(data->decoder, ARDecoder);
	AR_RELEASE(data,ARSimulatorCameraData);
}

bool ARSimulatorCamera::Start(){
	if( data->timer ){ return false; }
	data->delegate = [[NSTimerDelegate alloc] init];
	data->delegate->camera = this;
	double fps = frameRate;
	if( fps <= 0.0 ){
		data->timer = [NSTimer scheduledTimerWithTimeInterval:1.0/30.0 
													   target:data->delegate 
													 selector:@selector(onTimer) 
													 userInfo:nil repeats:NO];
	}else{
		data->timer = [NSTimer scheduledTimerWithTimeInterval:1.0/fps 
													   target:data->delegate 
													 selector:@selector(onTimer) 
													 userInfo:nil repeats:YES];
	}
	
	return true;
}

bool ARSimulatorCamera::Stop(){
	if( !data->timer ){ return false; }
	[data->timer invalidate];
	data->timer = NULL;
	[data->delegate release];
	data->delegate = NULL;
	return true;
}

bool ARSimulatorCamera::Pause(){
	return Stop();
}

bool ARSimulatorCamera::IsRunning(){
	return ( data->timer )? true : false;
}

bool ARSimulatorCamera::IsSimulator(){
	return true;
}

bool ARSimulatorCamera::SetSimulatorData(const char *path)
{
	AR_RELEASE(data->decoder,ARDecoder);
	char *ext = PathExtension(path);
	if( !ext ){ 
		return false;
	}
	if( CaseInsentiveEqual(ext,"png") || CaseInsentiveEqual(ext,"jpg") ){
		data->decoder = AR_ALLOC(ARImageDecoder);
	}else if( CaseInsentiveEqual(ext,"mov") || CaseInsentiveEqual(ext,"m4v") || CaseInsentiveEqual(ext,"mp4") ){
		data->decoder = AR_ALLOC(ARMovieDecoder);
	}
	data->decoder->SetSourceFile(path);
	int w = captureImage->width;
	int h = captureImage->height;
	int bpp = captureImage->bpp;
	LockBuffer();
	unsigned char *dst = buffers[1-activeIndex];
	data->decoder->Decode(w,h,bpp,dst);
	activeIndex = 1 - activeIndex;
	UnlockBuffer();
	return true;
}

void ARSimulatorCamera::OnTimer(){
	if( !data->decoder ){ return; }
	int w = captureImage->width;
	int h = captureImage->height;
	int bpp = captureImage->bpp;
	LockBuffer();
	unsigned char *dst = buffers[1-activeIndex];
	bool ret = data->decoder->Decode(w,h,bpp,dst);
	UnlockBuffer();
	if( ret ){
		activeIndex = 1 - activeIndex;
		if( callback ){ callback(dst,callbackArg); }
	}
}

static char* PathExtension(const char *path){
	if( !path ){ return NULL; }
	int l = strlen(path);
	for(int i=l-2;i>=0;i--){
		if( path[i] == '.' ){
			return (char*)&path[i+1];
		}
	}
	return NULL;
}


#include <ctype.h>
static bool CaseInsentiveEqual(const char *str1, const char *str2){
	for(int i=0;i<256;i++){
		int c1 = str1[i];
		int c2 = str2[i];
		if( c1 == 0 && c2 == 0 ){
			return true;
		}else if( c1 == 0 || c2 == 0 ){
			return false;
		}else if( tolower(c1)!=tolower(c2) ){ 
			return false;
		}
	}
	return false;
}

static bool HasSuffix(const char *str, const char *sfx){
	if( !str || !sfx ){ return false; }
	int l1 = strlen(str);
	int l2 = strlen(sfx);
	if( l2 > l1 ){ return false; }
	char *substr = (char*)str+(l1-l2);
	return strcmp(sfx,substr)==0 ? true : false;
}


