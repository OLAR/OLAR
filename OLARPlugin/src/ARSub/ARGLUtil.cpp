#include "ARGLUtil.h"
#include "cocos2d.h"

#if ANDROID
#include <GLES2/gl2.h>
#endif

#if __OBJC__
#	import <OpenGLES/EAGL.h>
#	import <OpenGLES/ES2/gl.h>
#	import <OpenGLES/ES2/glext.h>
#endif

#define VERTEX_SHADER_SOURCE \
"attribute vec4 position;"\
"attribute vec2 texcoord;"\
"varying vec2 v_TexCoord;"\
"void main(){"\
"gl_Position = position;"\
"v_TexCoord = texcoord;"\
"}"

#if ANDROID
#define FRAGMENT_SHADER_SOURCE \
"precision lowp float;"\
"varying lowp vec2 v_TexCoord;"\
"uniform sampler2D s_Texture;"\
"uniform sampler2D s_Texture2;"\
"const mat3 m = mat3(1.164,1.164,1.164, 0.0,-0.392,2.0, 1.596,-0.813,0.0 );"\
"const vec3 t = vec3(-0.8707,0.5295,-1.0813);"\
"void main(){"\
"gl_FragColor.rgb = m*vec3(texture2D(s_Texture,v_TexCoord).r,texture2D(s_Texture2,v_TexCoord).ar)+t;"\
"gl_FragColor.a = 1.0;"\
"}"
#else
#define FRAGMENT_SHADER_SOURCE \
"precision lowp float;"\
"varying lowp vec2 v_TexCoord;"\
"uniform sampler2D s_Texture;"\
"uniform sampler2D s_Texture2;"\
"const mat3 m = mat3(1.164,1.164,1.164, 0.0,-0.392,2.0, 1.596,-0.813,0.0 );"\
"const vec3 t = vec3(-0.8707,0.5295,-1.0813);"\
"void main(){"\
"gl_FragColor.rgb = m*vec3(texture2D(s_Texture,v_TexCoord).r,texture2D(s_Texture2,v_TexCoord).ra)+t;"\
"gl_FragColor.a = 1.0;"\
"}"
#endif

namespace ARGL{
	
	class YUVShader{
		bool initialized;
		uint32_t programId;
		uint32_t prevProgramId;
		uint32_t attributes[2];
		uint32_t uniforms[2];
	public:
		void Initialize(){
			if( initialized ){
				return;
			}
			prevProgramId = 0;
			programId = glCreateProgram();
			
			LoadShader(VERTEX_SHADER_SOURCE,GL_VERTEX_SHADER);
			LoadShader(FRAGMENT_SHADER_SOURCE,GL_FRAGMENT_SHADER);
			glLinkProgram(programId);
			GLint loglen;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &loglen);
			if( loglen > 0 ){
				char *log = (char *)malloc(loglen);
				glGetProgramInfoLog(programId, loglen, &loglen, log);
				free(log);
			}
			
			GLint status;
			glGetProgramiv(programId, GL_LINK_STATUS, &status);
			if( status == 0 ){
				return;
			}
			
			attributes[0] = GetAttribute("position");
			attributes[1] = GetAttribute("texcoord");
			uniforms[0]   = GetUniform("s_Texture");
			uniforms[1]   = GetUniform("s_Texture2");
			
			initialized = true;
		}

		void LoadShader(const char *source,uint32_t type){
			GLuint shaderId = glCreateShader(type);
			glShaderSource(shaderId, 1, &source, NULL);
			glCompileShader(shaderId);
			
			GLint loglen;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &loglen);
			if( loglen > 0 ){
				char *log = (char *)malloc(loglen);
				glGetShaderInfoLog(shaderId, loglen, &loglen, log);
				free(log);
			}
			
			GLint status = 0;
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
			if (status == 0){
				glDeleteShader(shaderId);
				return;
			}
			glAttachShader(programId, shaderId);
			glDeleteShader(shaderId);
		}
		
		uint32_t GetAttribute(const char *name){
			return glGetAttribLocation(programId,name);
		}
		
		uint32_t GetUniform(const char *name){
			return glGetUniformLocation(programId,name);
		}
		
	public:
		
		YUVShader(){
			initialized = false;
		}
		~YUVShader(){
			if( programId ){
				glDeleteProgram(programId);
			}
		}
		int attribFlags[2];
		void BindShader(uint32_t mainTex, uint32_t uvTex, float *v, float *t ){
			if( !initialized ){
				return;
			}
			for(int i=0;i<2;i++){
				glGetVertexAttribiv(attributes[i],GL_VERTEX_ATTRIB_ARRAY_ENABLED,&attribFlags[i]);
			}
			glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prevProgramId);
			
			glUseProgram(programId);
			
			glUniform1i(uniforms[0], 0); 
			glUniform1i(uniforms[1], 1);
			
			cocos2d::ccGLBindTexture2DN(0, mainTex);
			cocos2d::ccGLBindTexture2DN(1, uvTex);
			
			glEnableVertexAttribArray(attributes[0]);
			glEnableVertexAttribArray(attributes[1]);
			glVertexAttribPointer(attributes[0], 2, GL_FLOAT, 0, 0, v);
			glVertexAttribPointer(attributes[1], 2, GL_FLOAT, 0, 0, t);

		}
		
		void UnbindShader(){
			if( !initialized ){
				return;
			}
			cocos2d::ccGLBindTexture2DN(0, 0);
			cocos2d::ccGLBindTexture2DN(1, 0);
			for(int i=0;i<2;i++){
				if( !attribFlags[i] ){
					glDisableVertexAttribArray(attributes[i]);
				}
			}
			glUseProgram(prevProgramId);
		}
		
	};
	
	static YUVShader *yuvShader = NULL;
	
	int GetCurrentGLESVersion(){
#if ANDROID
		return 2;
#else
		return (int)[[EAGLContext currentContext] API];
#endif
	}
	
	void InitializeRenderer(){
		yuvShader = new YUVShader();
		yuvShader->Initialize();
	}
	
	void FinalizeRenderer(){
		if(yuvShader){
			delete yuvShader;
			yuvShader=NULL;
		}
	}
	
	void RenderYUVImage(uint32_t mainTex, uint32_t uvTex){
		static float t[8] = { 1,1, 1,0, 0,0, 0,1};
		RenderYUVImage(mainTex, uvTex, t);
	}
	
	void RenderYUVImage(uint32_t mainTex, uint32_t uvTex, float *texcoord){
		static float v[8] = {-1,-1,+1,-1,+1,+1,-1,+1};
		static unsigned char indices[4] = {0,1,3,2};
		glDepthMask(0);
		yuvShader->BindShader(mainTex, uvTex, v, texcoord);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);
		yuvShader->UnbindShader();
		glDepthMask(1);
	}
	
};
