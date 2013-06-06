#ifndef __OLAR_AR_GLUTIL_H__
#define __OLAR_AR_GLUTIL_H__

#include <stdint.h>

#if ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif __OBJC__
#import <Foundation/Foundation.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#endif

namespace ARGL{
	int  GetCurrentGLESVersion();
	void InitializeRenderer();
	void FinalizeRenderer();
	void RenderYUVImage(uint32_t mainTex, uint32_t uvTex);
	void RenderYUVImage(uint32_t mainTex, uint32_t uvTex, float *texcoord);
};

#endif

