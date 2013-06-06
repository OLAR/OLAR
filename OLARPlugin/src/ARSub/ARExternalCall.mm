#include "ARExternalCall.h"
#include <dlfcn.h>
#include <objc/objc.h>
#include <objc/runtime.h>

#define DYLIB_LOAD(tgt,sym) ({ tgt = (typeof(tgt))dlsym(dl,(sym)); })
#define EXPORT_CLASS(cls) static Class cls##_=NULL; Class cls(){ return cls##_; }

static BOOL isLoaded = FALSE;
static CVImageBufferRef (*__CMSampleBufferGetImageBuffer)(CMSampleBufferRef sbuf) = NULL;
static CVReturn (*__CVPixelBufferLockBaseAddress)(CVPixelBufferRef pixelBuffer, CVOptionFlags lockFlags)=NULL;
static void* (*__CVPixelBufferGetBaseAddress)(CVPixelBufferRef pixelBuffer) = NULL;
static size_t (*__CVPixelBufferGetBytesPerRow)(CVPixelBufferRef pixelBuffer) = NULL;
static size_t (*__CVPixelBufferGetWidth)(CVPixelBufferRef pixelBuffer) = NULL;
static size_t (*__CVPixelBufferGetHeight)(CVPixelBufferRef pixelBuffer) = NULL;
static void* (*__CVPixelBufferGetBaseAddressOfPlane)(CVPixelBufferRef pixelBuffer,size_t index) = NULL;
static size_t (*__CVPixelBufferGetBytesPerRowOfPlane)(CVPixelBufferRef pixelBuffer,size_t index) = NULL;
static size_t (*__CVPixelBufferGetWidthOfPlane)(CVPixelBufferRef pixelBuffer,size_t index) = NULL;
static size_t (*__CVPixelBufferGetHeightOfPlane)(CVPixelBufferRef pixelBuffer,size_t index) = NULL;
static CVReturn (*__CVPixelBufferUnlockBaseAddress)(CVPixelBufferRef pixelBuffer, CVOptionFlags unlockFlags) = NULL;
static void (*__CVPixelBufferRelease)( CVPixelBufferRef pixelBuffer ) = NULL;

namespace ARExternalCall{
	
	class AutoLoad{
	public: AutoLoad(){
			ARExternalCall::Initialize();
		}
	};
	static AutoLoad __autoload;
	
	EXPORT_CLASS(AVCaptureSession);
	EXPORT_CLASS(AVCaptureDevice);
	EXPORT_CLASS(AVCaptureDeviceInput);
	EXPORT_CLASS(AVCaptureVideoDataOutput);
	EXPORT_CLASS(AVCaptureVideoPreviewLayer);
	EXPORT_CLASS(AVURLAsset);
	EXPORT_CLASS(AVAssetReader);
	EXPORT_CLASS(AVAssetReaderTrackOutput);
	
	void Initialize(){
		if( isLoaded ){ return; }
		void *dl = NULL;
		
		dl = dlopen("/System/Library/Frameworks/CoreMedia.framework/CoreMedia",RTLD_LOCAL);
		if ( dl ){ 
			DYLIB_LOAD(__CMSampleBufferGetImageBuffer,"CMSampleBufferGetImageBuffer");
			dlclose(dl);
		}
		
		dl = dlopen("/System/Library/Frameworks/CoreVideo.framework/CoreVideo",RTLD_LOCAL);
		if( dl ){
			DYLIB_LOAD(__CVPixelBufferLockBaseAddress,"CVPixelBufferLockBaseAddress");
			DYLIB_LOAD(__CVPixelBufferGetBaseAddress,"CVPixelBufferGetBaseAddress");
			DYLIB_LOAD(__CVPixelBufferGetBytesPerRow,"CVPixelBufferGetBytesPerRow");
			DYLIB_LOAD(__CVPixelBufferGetWidth,"CVPixelBufferGetWidth");
			DYLIB_LOAD(__CVPixelBufferGetHeight,"CVPixelBufferGetHeight");
			DYLIB_LOAD(__CVPixelBufferGetBaseAddressOfPlane,"CVPixelBufferGetBaseAddressOfPlane");
			DYLIB_LOAD(__CVPixelBufferGetBytesPerRowOfPlane,"CVPixelBufferGetBytesPerRowOfPlane");
			DYLIB_LOAD(__CVPixelBufferGetWidthOfPlane,"CVPixelBufferGetWidthOfPlane");
			DYLIB_LOAD(__CVPixelBufferGetHeightOfPlane,"CVPixelBufferGetHeightOfPlane");
			DYLIB_LOAD(__CVPixelBufferUnlockBaseAddress,"CVPixelBufferUnlockBaseAddress");
			DYLIB_LOAD(__CVPixelBufferRelease,"CVPixelBufferRelease");
			dlclose(dl);
		}
		
		dl = dlopen("/System/Library/Frameworks/AVFoundation.framework/AVFoundation",RTLD_LOCAL);
		if( dl ){
			AVCaptureSession_ = (id)objc_getClass("AVCaptureSession");
			AVCaptureDevice_ = (id)objc_getClass("AVCaptureDevice");
			AVCaptureDeviceInput_ = (id)objc_getClass("AVCaptureDeviceInput");
			AVCaptureVideoDataOutput_ = (id)objc_getClass("AVCaptureVideoDataOutput");
			AVCaptureVideoPreviewLayer_ = (id)objc_getClass("AVCaptureVideoPreviewLayer");
			AVURLAsset_ = (id)objc_getClass("AVURLAsset");
			AVAssetReader_ = (id)objc_getClass("AVAssetReader");
			AVAssetReaderTrackOutput_ = (id)objc_getClass("AVAssetReaderTrackOutput");			
			dlclose(dl);
		}
		
		isLoaded = true;
	}
	
	CVImageBufferRef CMSampleBufferGetImageBuffer(CMSampleBufferRef sbuf){
		return __CMSampleBufferGetImageBuffer(sbuf);
	}
	
	void   CVPixelBufferLockBaseAddress(CVPixelBufferRef pixelBuffer,CVOptionFlags lockFlags){
		__CVPixelBufferLockBaseAddress(pixelBuffer,lockFlags);
	}
	
	void   CVPixelBufferUnlockBaseAddress(CVPixelBufferRef pixelBuffer,CVOptionFlags unlockFlags){
		__CVPixelBufferUnlockBaseAddress(pixelBuffer,unlockFlags);
	}
	
	void*  CVPixelBufferGetBaseAddress(CVPixelBufferRef pixelBuffer){
		return __CVPixelBufferGetBaseAddress(pixelBuffer);
	}
	
	size_t CVPixelBufferGetBytesPerRow(CVPixelBufferRef pixelBuffer){
		return __CVPixelBufferGetBytesPerRow(pixelBuffer);
	}
	
	size_t CVPixelBufferGetWidth(CVPixelBufferRef pixelBuffer){
		return __CVPixelBufferGetWidth(pixelBuffer);
	}
	
	size_t CVPixelBufferGetHeight(CVPixelBufferRef pixelBuffer){
		return __CVPixelBufferGetHeight(pixelBuffer);
	}
	
	void*  CVPixelBufferGetBaseAddressOfPlane(CVPixelBufferRef pixelBuffer,size_t index){
		return __CVPixelBufferGetBaseAddressOfPlane(pixelBuffer,index);
	}
	
	size_t CVPixelBufferGetBytesPerRowOfPlane(CVPixelBufferRef pixelBuffer,size_t index){
		return __CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer,index);
	}
	
	size_t CVPixelBufferGetWidthOfPlane(CVPixelBufferRef pixelBuffer, size_t index){
		return __CVPixelBufferGetWidthOfPlane(pixelBuffer,index);
	}
	
	size_t CVPixelBufferGetHeightOfPlane(CVPixelBufferRef pixelBuffer, size_t index){
		return __CVPixelBufferGetHeightOfPlane(pixelBuffer,index);
	}
	
	void   CVPixelBufferRelease(CVPixelBufferRef pixelBuffer){
		__CVPixelBufferRelease(pixelBuffer);
	}
		
	
	
}