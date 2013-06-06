#include <UIKit/UIKit.h>
#include <CoreGraphics/CoreGraphics.h>
#include "ARCommon.h"
#include "ARDecoder.h"
#include "ARConfig.h"
#include "ARExternalCall.h"

static bool GetAutoRotatedPixelBufferFromCGImage(uint8_t *dst, int w, int h, int bpp,CGImageRef cgImage);

/////////////////////////////////
//       Image Decoder         //
/////////////////////////////////

ARImageDecoder::ARImageDecoder(){
	srcImage = NULL;
}

ARImageDecoder::~ARImageDecoder(){
	if( srcImage ){
		CGImageRelease((CGImageRef)srcImage);
	}
}

bool ARImageDecoder::SetSourceFile(const char *path){
	CGImageRef cgImage = [UIImage imageNamed:[NSString stringWithUTF8String:path]].CGImage;
	if( !cgImage ){ return false; }
	CGImageRetain(cgImage);
	srcImage = cgImage;
	return true;
}

bool ARImageDecoder::Decode(int w, int h, int bpp, unsigned char *dst){
	if( !srcImage ){
		return false;
	}
	return GetAutoRotatedPixelBufferFromCGImage(dst,w,h,bpp,(CGImageRef)srcImage);
}

/////////////////////////////////
//       Movie Decoder         //
/////////////////////////////////

typedef struct MovieInfoT{
	int width;
	int height;
	AVURLAsset    *asset;
	AVAssetReader *assetReader;
	AVAssetReaderTrackOutput *assetReaderOutput;
}MovieInfo;

static MovieInfo* AllocMovieInfo(const char *path){
	AVURLAsset *asset = NULL;
	NSString *path2 = [NSString stringWithUTF8String:path];
	
	if( ![[NSFileManager defaultManager] fileExistsAtPath:path2] ){
		path2 = [[NSBundle mainBundle] pathForResource:[path2 stringByDeletingPathExtension] ofType:[path2 pathExtension]];
		if( ![[NSFileManager defaultManager] fileExistsAtPath:path2] ){
			return NULL;
		}
	}
	NSURL *url = [NSURL fileURLWithPath:path2];
	asset = [[ARExternalCall::AVURLAsset() alloc] initWithURL:url options:nil];
	
	MovieInfo *movieInfo = AR_ALLOC(MovieInfo);
	movieInfo->asset = asset;
	
	AVAssetTrack *track = [[movieInfo->asset tracksWithMediaType:AVMediaTypeVideo] objectAtIndex:0];
	movieInfo->width = (int)track.naturalSize.width;
	movieInfo->height = (int)track.naturalSize.height;
	
	NSDictionary *setting = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:'BGRA'] forKey:(id)@"PixelFormatType"];
	movieInfo->assetReaderOutput = [[ARExternalCall::AVAssetReaderTrackOutput() alloc] initWithTrack:track outputSettings:setting];
	movieInfo->assetReader = [[ARExternalCall::AVAssetReader() alloc] initWithAsset:movieInfo->asset error:nil];
	[movieInfo->assetReader addOutput:movieInfo->assetReaderOutput];
	[movieInfo->assetReaderOutput release];
	[movieInfo->assetReader startReading];
	return movieInfo;
}

static void ReleaseMovieInfo(MovieInfo *movieInfo){
	if( !movieInfo ){ return; }
	[movieInfo->asset release];
	[movieInfo->assetReader release];
	[movieInfo->assetReaderOutput release];
	movieInfo->asset = NULL;
	movieInfo->assetReader = NULL;
	movieInfo->assetReaderOutput = NULL;
	AR_RELEASE(movieInfo,MovieInfo);
}

ARMovieDecoder::ARMovieDecoder(){
	srcMovie = NULL;
}

ARMovieDecoder::~ARMovieDecoder(){
	if( srcMovie ){
		ReleaseMovieInfo((MovieInfo*)srcMovie);
	}
}

bool ARMovieDecoder::SetSourceFile(const char *path){
	if( srcMovie ){
		ReleaseMovieInfo((MovieInfo*)srcMovie);
		srcMovie = NULL;
	}
	srcMovie = AllocMovieInfo(path);
	return true;
}

bool ARMovieDecoder::Decode(int w, int h, int bpp, unsigned char *dst){
	if( !srcMovie ){
		return false;
	}
	
	AVAssetReader *reader = srcMovie->assetReader;
	if( reader.status != AVAssetReaderStatusReading ){
		if( reader.status == AVAssetReaderStatusCompleted ){
			
		}
		return false;
	}
	CMSampleBufferRef sampleBuffer = [srcMovie->assetReaderOutput copyNextSampleBuffer];
	if( !sampleBuffer ){ return false; }
	
	CVImageBufferRef pixBuff = ARExternalCall::CMSampleBufferGetImageBuffer(sampleBuffer); 
	ARExternalCall::CVPixelBufferLockBaseAddress(pixBuff, 0);
	void *buff = ARExternalCall::CVPixelBufferGetBaseAddress(pixBuff);
	
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreateWithData(buff,srcMovie->width,srcMovie->height,
											8,4*srcMovie->width,cs,kCGBitmapByteOrderDefault|kCGImageAlphaNoneSkipLast,NULL,NULL);
	CGImageRef cgImage = CGBitmapContextCreateImage(context);
	CGContextRelease(context);
	CGColorSpaceRelease(cs);
	
	ARExternalCall::CVPixelBufferUnlockBaseAddress(pixBuff,0);
	ARExternalCall::CVPixelBufferRelease(pixBuff);
	
	bool ret = GetAutoRotatedPixelBufferFromCGImage(dst,w,h,bpp,cgImage);
	CGImageRelease(cgImage);
	
	return ret;
}


static bool GetAutoRotatedPixelBufferFromCGImage(uint8_t *dst, int w, int h, int bpp,CGImageRef cgImage)
{	
	if( !cgImage ){
		return false;
	}
	double sa = (double)CGImageGetWidth(cgImage)/CGImageGetHeight(cgImage);
	double da = (double)w/h;
	
	unsigned char *buff = AR_ALLOC_ARRAY(unsigned char, (w*h*4));
	CGBitmapInfo binfo = kCGImageAlphaNoneSkipFirst|kCGBitmapByteOrder32Little;
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGContextRef ctx = CGBitmapContextCreate(buff,w,h,8,w*4,cs,binfo);
	if( (sa > 1.0 && da < 1.0) || (sa < 1.0 && da > 1.0) ){
		CGContextTranslateCTM(ctx, +0.5*w, +0.5*h);
		CGContextRotateCTM(ctx, M_PI*0.5);
		CGContextTranslateCTM(ctx, -0.5*h, -0.5*w);
		CGContextDrawImage(ctx,CGRectMake(0,0,h,w),cgImage);
	}else{
		CGContextDrawImage(ctx,CGRectMake(0,0,w,h),cgImage);
	}
	CGColorSpaceRelease(cs);
	CGContextRelease(ctx);
	
	if( bpp == 1 ){
		unsigned char *s = buff;
		unsigned char *d = dst;
		for(int i=0;i<(w*h);i++){
			*d = (unsigned char)(((int)*(s+0)+((int)*(s+1))+(int)*(s+2))/3);
			s+=4;
			d++;
		}
		const int n2 = w*h/4;
		d = dst + (w*h);
		for(int i=0;i<n2;i++){
			*(d+0) = 128;
			*(d+1) = 128;
			d+=2;
		}
	}else if( bpp == 4 ){
		unsigned char *s = buff;
		unsigned char *d = dst;
		for(int i=0;i<(w*h);i++){
			*(d+0) = *(s+0);
			*(d+1) = *(s+1);
			*(d+2) = *(s+2);
			*(d+3) = *(s+3);
			s+=4;
			d+=4;
		}
	}
	return true;
}
