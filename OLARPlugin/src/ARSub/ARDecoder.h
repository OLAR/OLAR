#ifndef __OLAR_AR_DECODER_H__
#define __OLAR_AR_DECODER_H__

class ARDecoder{
protected:
public:
	virtual ~ARDecoder(){};
	virtual bool SetSourceFile(const char *path) = 0;
	virtual bool Decode(int w, int h, int fmt,unsigned char *dst) = 0;
};

class ARImageDecoder : public ARDecoder{
	void *srcImage;
public:
	ARImageDecoder();
	virtual ~ARImageDecoder();
	virtual bool SetSourceFile(const char *path);
	virtual bool Decode(int w, int h, int fmt,unsigned char *dst);
};

class ARMovieDecoder : public ARDecoder{
	struct MovieInfoT *srcMovie;
public:
	ARMovieDecoder();
	virtual ~ARMovieDecoder();
	virtual bool SetSourceFile(const char *path);
	virtual bool Decode(int w, int h, int fmt,unsigned char *dst);
};

#endif
