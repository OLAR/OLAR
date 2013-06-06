#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

namespace ARExternalCall {
	void   Initialize();
	CVImageBufferRef CMSampleBufferGetImageBuffer(CMSampleBufferRef sbuf);
	void   CVPixelBufferLockBaseAddress(CVPixelBufferRef pixelBuffer,CVOptionFlags lockFlags);
	void   CVPixelBufferUnlockBaseAddress(CVPixelBufferRef pixelBuffer,CVOptionFlags unlockFlags);
	void*  CVPixelBufferGetBaseAddress(CVPixelBufferRef pixelBuffer);
	size_t CVPixelBufferGetBytesPerRow(CVPixelBufferRef pixelBuffer);
	size_t CVPixelBufferGetWidth(CVPixelBufferRef pixelBuffer);
	size_t CVPixelBufferGetHeight(CVPixelBufferRef pixelBuffer);
	void*  CVPixelBufferGetBaseAddressOfPlane(CVPixelBufferRef pixelBuffer,size_t index);
	size_t CVPixelBufferGetBytesPerRowOfPlane(CVPixelBufferRef pixelBuffer,size_t index);
	size_t CVPixelBufferGetWidthOfPlane(CVPixelBufferRef pixelBuffer, size_t index);
	size_t CVPixelBufferGetHeightOfPlane(CVPixelBufferRef pixelBuffer, size_t index);
	void   CVPixelBufferRelease(CVPixelBufferRef pixelBuffer);
	CMTime CMTimeMake(int v1,int v2);
	Class  AVCaptureSession();
	Class  AVCaptureDevice();
	Class  AVCaptureDeviceInput();
	Class  AVCaptureVideoDataOutput();
	Class  AVCaptureVideoPreviewLayer();
	Class  AVURLAsset();
	Class  AVAssetReader();
	Class  AVAssetReaderTrackOutput();
};
