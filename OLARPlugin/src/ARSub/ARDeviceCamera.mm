#if !TARGET_IPHONE_SIMULATOR
#define AR_CAMERA_PRIVATE

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "ARCamera.h"
#import "ARExternalCall.h"

#define AVMediaTypeVideo_ @"vide"

@interface ARDeviceCameraDelegate : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
{
@public
	ARDeviceCamera *camera;
}
@end

ARDeviceCamera::ARDeviceCamera() : ARCamera(){
	session = NULL;
	delegate = [[ARDeviceCameraDelegate alloc] init];
	((ARDeviceCameraDelegate*)delegate)->camera = this;
}

ARDeviceCamera::~ARDeviceCamera(){
	[(AVCaptureSession*)session release];
	[(ARDeviceCameraDelegate*)delegate release];
}

bool ARDeviceCamera::SetConfig(ARConfig *config)
{
	ARExternalCall::Initialize();
	//ARExternalCall::initialize];
	
	ARCamera::SetConfig(config);
	
	frameRate = 0;
	AVCaptureSession *_session = (AVCaptureSession*)session;
	if( _session ){
		if( [_session isRunning] ){
			[_session stopRunning];
		}
		[_session release];
	}	
	
	NSString *preset = NULL;
	if( config->sizePreset == ARCameraSize480x360 ){
		preset = @"AVCaptureSessionPresetMedium";
	}else if( config->sizePreset == ARCameraSize640x480 ){
		preset = @"AVCaptureSessionPreset640x480";
	}else{
		preset = @"AVCaptureSessionPreset640x480";
	}
	
	Class AVCaptureSession_ = ARExternalCall::AVCaptureSession();
	Class AVCaptureDevice_ = ARExternalCall::AVCaptureDevice();
	Class AVCaptureDeviceInput_ = ARExternalCall::AVCaptureDeviceInput();
	Class AVCaptureVideoDataOutput_ = ARExternalCall::AVCaptureVideoDataOutput();
	
	_session = [[AVCaptureSession_ alloc] init];
	_session.sessionPreset = preset;
	session = _session;
	
	NSError *error = nil;
	AVCaptureDevice *device  = [AVCaptureDevice_ defaultDeviceWithMediaType:AVMediaTypeVideo_];
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput_ deviceInputWithDevice:device error:&error];
	AVCaptureVideoDataOutput *output = [[[AVCaptureVideoDataOutput_ alloc] init] autorelease];
	
	//[device lockForConfiguration:NULL];
	//[device setFocusMode:AVCaptureFocusModeLocked];
	//[device unlockForConfiguration];
	
	// kCVPixelFormatType_32BGRA = 0x42475241 : BGRA32
	// kCVPixelFormatType_420YpCbCr8BiPlanarFullRange = 0x34323066 : 420YpCbCr8
	NSNumber *internalPixformat = NULL;	
	if( config->pixelFormat == ARCameraFormatBGRA ){
		internalPixformat = [NSNumber numberWithInt:(int)('BGRA')];
	}else{
		internalPixformat = [NSNumber numberWithInt:(int)('420v')];
	}
	output.videoSettings = [NSDictionary dictionaryWithObject:internalPixformat 
													   forKey:(id)@"PixelFormatType"];
	
	output.alwaysDiscardsLateVideoFrames = YES;
	
	[_session addInput:input];
	[_session addOutput:output];
	
	dispatch_queue_t queue = dispatch_get_main_queue();
	[output setSampleBufferDelegate:(ARDeviceCameraDelegate*)delegate queue:queue];
    dispatch_release(queue);
		
	return TRUE;
}

bool ARDeviceCamera::IsRunning(){
	return ( session && [(AVCaptureSession*)session isRunning] )? true : false;
}

bool ARDeviceCamera::Start(){
	if( !session || [(AVCaptureSession*)session isRunning] ){ return false; }
	captured = NO;
	[(AVCaptureSession*)session startRunning];
	return TRUE;
}

bool ARDeviceCamera::Stop(){
	if( !session ){ return false; }
	[(AVCaptureSession*)session stopRunning];
	return TRUE; 
}

bool ARDeviceCamera::Pause(){
	if( !session ){ return false; }
	[(AVCaptureSession*)session stopRunning];
	return true;
}

bool ARDeviceCamera::IsSimulator(){
	return false;
}

void ARDeviceCamera::OnCaptureFrame(void *buffer)
{
	CMSampleBufferRef buffer2 = (CMSampleBufferRef)buffer;
	CVImageBufferRef pixBuff = ARExternalCall::CMSampleBufferGetImageBuffer(buffer2);
	ARExternalCall::CVPixelBufferLockBaseAddress(pixBuff,0);
		
	void *yplane = NULL, *uvplane = NULL;
	if( currentConfig.pixelFormat == ARCameraFormatYUVSP ){
		yplane = ARExternalCall::CVPixelBufferGetBaseAddressOfPlane(pixBuff,0);
		if( !captured ){
			int w = ARExternalCall::CVPixelBufferGetWidthOfPlane(pixBuff,0);
			int h = ARExternalCall::CVPixelBufferGetHeightOfPlane(pixBuff,0);
			int p = ARExternalCall::CVPixelBufferGetBytesPerRowOfPlane(pixBuff,0);
			if( w!=captureImage->width || h!=captureImage->height || p!=captureImage->pitch ){
				NSLog(@"[ARDeviceCamera]: FATAL ERROR!!! Invalid pixel buffer size (%dx%d %d)",w,h,(int)(p/w));
				return;
			}
			NSLog(@"[ARDeviceCamera]: capture started [YUV] (%dx%d)",w,h);
			captured = YES;
		}
		uvplane = ARExternalCall::CVPixelBufferGetBaseAddressOfPlane(pixBuff,1);
	}else if( currentConfig.pixelFormat == ARCameraFormatBGRA ){
		yplane = ARExternalCall::CVPixelBufferGetBaseAddress(pixBuff);
		if( !captured ){
			int w = ARExternalCall::CVPixelBufferGetWidth(pixBuff);
			int h = ARExternalCall::CVPixelBufferGetHeight(pixBuff);
			int p = ARExternalCall::CVPixelBufferGetBytesPerRow(pixBuff);
			if( w!=captureImage->width || h!=captureImage->height || p!=captureImage->pitch ){
				NSLog(@"[ARDeviceCamera]: FATAL ERROR!!! Invalid pixel buffer size (%dx%d %d)",w,h,(int)(p/w));
				return;
			}
			NSLog(@"[ARDeviceCamera]: capture started [BGRA] (%dx%d)",w,h);
			captured = YES;
		}
	}
	
	//[ARStopWatch start:@"BufferCopy"];
	LockBuffer();
	int size = (captureImage->height*captureImage->pitch);
	unsigned char *dst = buffers[1-activeIndex];
	memcpy(dst,yplane,size);
	if( uvplane ){ memcpy(dst+size,uvplane,size/2); }
	activeIndex = 1 - activeIndex;
	UnlockBuffer();
	//[ARStopWatch stop:@"BufferCopy"];
	
	ARExternalCall::CVPixelBufferUnlockBaseAddress(pixBuff,0);
	if( callback ){ callback(dst,callbackArg); }
}


@implementation ARDeviceCameraDelegate

- (void)captureOutput:(AVCaptureOutput *)output 
		didOutputSampleBuffer:(CMSampleBufferRef)buffer
		fromConnection:(AVCaptureConnection *)connection
{
	if( camera ){
		camera->OnCaptureFrame(buffer);
	}
}

@end

#endif
