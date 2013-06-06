#include "ARCommon.h"

#if ANDROID

int ARGetDeviceOrientation(jobject activity){
	if( activity == NULL ){
		return AROrientationUnknown;
	}
	JNIEnv *env = getJNIEnv();
	if( env == NULL ){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return AROrientationUnknown;
	}
	jclass klass = env->GetObjectClass(activity);
	if( klass == NULL ){
		OLARDUMP("%s:GetObjectClass is null",__FUNCTION__);
		return AROrientationUnknown;
	}
	jmethodID mID = env->GetMethodID(klass,"getDeviceOrientation","()I");
	if( mID == NULL ){
		OLARDUMP("%s:GetMethodID is null",__FUNCTION__);
		return AROrientationUnknown;
	}
	return (int)env->CallIntMethod(activity,mID);
}

uint8_t* ARLoadDataFromFile(jobject activity,const char *path,int *length){
	*length = 0;
	if(activity==NULL)return NULL;
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return NULL;
	}
	jclass klass = env->GetObjectClass(activity);
	if(klass==NULL){
		OLARDUMP("%s:GetObjectClass is null",__FUNCTION__);
		return NULL;
	}
	jmethodID mID = env->GetMethodID(klass,"loadAssetsData","(Ljava/lang/String;)[B");
	if(mID==NULL){
		OLARDUMP("%s:GetMethodID is null",__FUNCTION__);
		return NULL;
	}
	
	jbyte *data = NULL;
	
	jstring jPath = env->NewStringUTF(path);
	jbyteArray jData = static_cast<jbyteArray>(env->CallObjectMethod(activity,mID,jPath));
	if(jData!=NULL){
		int len = env->GetArrayLength(jData);
		if(len>0){
			data = reinterpret_cast<jbyte *>(AR_ALLOC_ARRAY(uint8_t,len));
			env->GetByteArrayRegion(jData,0,len,data);
			*length = len;
		}
	}
	env->ReleaseStringUTFChars(jPath,NULL);
	
	//getPackageCodePath()
	
	OLARDUMP("ARLoadDataFromFile: %p",data);
	
	return reinterpret_cast<uint8_t *>(data);
}

#else

int ARGetDeviceOrientation()
{
	UIInterfaceOrientation ori = [[UIApplication sharedApplication] statusBarOrientation];
	switch(ori){
		case UIInterfaceOrientationPortrait:
			return (int)AROrientationPortrait;
		case UIInterfaceOrientationPortraitUpsideDown:
			return (int)AROrientationPortraitUpsideDown;
		case UIInterfaceOrientationLandscapeLeft:
			return (int)AROrientationLandscapeLeft;
		case UIInterfaceOrientationLandscapeRight:
			return (int)AROrientationLandscapeRight;
		default:
			return (int)AROrientationUnknown;
	}
}

uint8_t* ARLoadDataFromFile(const char *path,int *length)
{
	FILE *fp = fopen(path,"rb");
	if( !fp ){
		NSString *path2 = [NSString stringWithUTF8String:path];
		NSString *name = [path2 stringByDeletingPathExtension];
		NSString *ext  = [path2 pathExtension];
		NSString *path3 = [[NSBundle mainBundle] pathForResource:name ofType:ext];
		fp = fopen([path3 UTF8String],"rb");
		if( !fp ){
			if( length ){ *length = 0; }
			return NULL;
		}
		path = [path3 UTF8String];
	}
	OLARDUMP("Load: [%s]",path);
	fseek(fp, 0, SEEK_END);
	int l = (int)ftell(fp);
	if( length ){ *length = l; }
	if( l <= 0 ){
		return NULL;
	}
	uint8_t *buff = AR_ALLOC_ARRAY(uint8_t,l);
	fseek(fp, 0, SEEK_SET);
	fread(buff, l, 1, fp);
	fclose(fp);
	return buff;
}
#endif

#if ANDROID
JavaVM *g_JavaVM;
void setJavaVM(JavaVM *vm){
	g_JavaVM = vm;
}
JavaVM *getJavaVM(){
	return g_JavaVM;
}
JNIEnv* getJNIEnv(){
	JavaVM *vm = getJavaVM();
	if (vm==NULL) {
		return NULL;
	}
	JNIEnv *env=NULL;
	if(vm->GetEnv(reinterpret_cast<void **>(&env),JNI_VERSION_1_4)==JNI_OK){
		return env;
	}
	return NULL;
}
JNIEnv* getJNIEnvX(int *attached){
	*attached=0;
	JavaVM *vm = getJavaVM();
	if (vm==NULL) {
		return NULL;
	}
	JNIEnv *env=NULL;
	if(vm->GetEnv(reinterpret_cast<void**>(&env),JNI_VERSION_1_4)==JNI_OK){
		return env;
	}
	if(vm->AttachCurrentThread(&env, NULL)==JNI_OK){
		*attached = 1;
		return env;
	}
	return NULL;
}
#endif

QMPoint::QMPoint():m_X(0),m_Y(0){}
QMPoint::QMPoint(float x,float y):m_X(x),m_Y(y){}
float QMPoint::x()const{ return m_X; }
void QMPoint::setX(float x){ m_X = x;}
float QMPoint::y()const{ return m_Y; }
void QMPoint::setY(float y){ m_Y = y;}

char *QMPoint::cstr() const{
	char *ret = AR_ALLOC_ARRAY(char,100);
	snprintf(ret,100,"<%f,%f>",m_X,m_Y);
	return ret;
}

QMSize::QMSize():m_Width(0),m_Height(0){}
QMSize::QMSize(float w,float h):m_Width(w),m_Height(h){}
float QMSize::width()const{return m_Width;}
float QMSize::height()const{return m_Height;}
char *QMSize::cstr() const{
	char *ret = AR_ALLOC_ARRAY(char,100);
	snprintf(ret,100,"(%fx%f)",m_Width,m_Height);
	return ret;
}


QMRect::QMRect():m_Left(0),m_Top(0),m_Width(0),m_Height(0){}
QMRect::QMRect(float l,float t,float w,float h):m_Left(l),m_Top(t),m_Width(w),m_Height(h){}
float QMRect::left()const{return m_Left;}
float QMRect::top()const{return m_Top;}
float QMRect::width()const{return m_Width;}
float QMRect::height()const{return m_Height;}
float QMRect::right()const{return m_Left+m_Width;}
float QMRect::bottom()const{return m_Top+m_Height;}
QMPoint QMRect::center()const{return QMPoint(m_Left + m_Width/2.0,m_Top + m_Height/2.0);}
QMSize QMRect::size()const{return QMSize(m_Width,m_Height);}

QMRect QMRect::aspectWidth(const QMSize &size)const{
	float w = m_Width;
	float h = w * size.height() / size.width();
	float l = m_Left;
	float t = m_Top + m_Height/2.0 - h/2.0;
	return QMRect(l,t,w,h);
}
QMRect QMRect::aspectHeight(const QMSize &size)const{
	float h = m_Height;
	float w = h * size.width() / size.height();
	float t = m_Top;
	float l = m_Left + m_Width/2.0 - w/2.0;
	return QMRect(l,t,w,h);
}
QMRect QMRect::aspectFit(const QMSize &size)const{
	if(m_Width * size.height() > size.width() * m_Height){
		return aspectHeight(size);
	}else{
		return aspectWidth(size);
	}
}

char *QMRect::cstr() const{
	char *ret = AR_ALLOC_ARRAY(char,100);
	snprintf(ret,100,"<%f,%f>-(%fx%f)",m_Left,m_Top,m_Width,m_Height);
	return ret;
}

char *CStrFromMatrix4x4(float *m){
	char *ret = AR_ALLOC_ARRAY(char,300);
	
	snprintf(ret,300,
			 "[%6.3f %6.3f %6.3f %6.3f\n"
			 " %6.3f %6.3f %6.3f %6.3f\n"
			 " %6.3f %6.3f %6.3f %6.3f\n"
			 " %6.3f %6.3f %6.3f %6.3f]",
			 m[0],m[4],m[8],m[12],
			 m[1],m[5],m[9],m[13],
			 m[2],m[6],m[10],m[14],
			 m[3],m[7],m[11],m[15]);
	
	return ret;
}



OLARArray::OLARArray()
{
	count = 0;
	items = NULL;
	capacity = 0;
	capacity0 = OLAR_ARRAY_DEFAULT_CAPACITY;
	ResizeCapacity(capacity0);
	Resize(0);
}

OLARArray::OLARArray(int _count)
{
	count = 0;
	items = NULL;
	capacity = 0;
	if( _count < OLAR_ARRAY_DEFAULT_CAPACITY ){
		capacity0 = OLAR_ARRAY_DEFAULT_CAPACITY;
	}else{
		capacity0 = _count;
	}
	ResizeCapacity(capacity0);
	Resize(_count);
}

OLARArray::~OLARArray(){
	Release();
}

void** OLARArray::Items(){ return items; }

int OLARArray::Count(){ return count; }

bool OLARArray::Add(void *item)
{
	if( count == capacity ){
		ResizeCapacity(count+capacity0);
	}
	items[count++] = item;
	return true;
}

bool OLARArray::Remove(int index)
{
	if( index < 0 || index >= count ){
		return false;
	}
	for(int i=index;i<count-1;i++){
		items[i] = items[i+1];
	}
	items[--count] = NULL;
	return true;
}

bool OLARArray::Remove(void *item)
{
	int index = -1;
	for(int i=0;i<count;i++){
		if( items[i] == item ){
			index = i;
			break;
		}
	}
	return Remove(index);
}

void* OLARArray::ItemAt(int index)
{
	if( index < 0 || index >= count ){
		return NULL;
	}
	return items[index];
}

int OLARArray::IndexOf(void *item){
	for(int i=0;i<count;i++){
		if( items[i] == item ){
			return i;
		}
	}
	return -1;
}

void OLARArray::Clear()
{
	count = 0;
	ResizeCapacity(capacity0);
}

void OLARArray::Release()
{
	count = 0;
	capacity = 0;
	if( items ){ delete [] items; }
	items = NULL;
}

void OLARArray::Resize(int _count)
{
	if( !items ){ return; }
	if( _count < 0 ){ _count = 0; }
	if( _count <= capacity ){
		count = _count;
		return;
	}
	if( _count > capacity ){
		ResizeCapacity(_count);
	}
	count = _count;
}

void OLARArray::ResizeCapacity(int _capacity)
{
	if( _capacity < capacity0 ){ _capacity = capacity0; }
	void **items0 = items;
	items = new void*[_capacity];
	memset(items,0,sizeof(void*)*_capacity);
	if( items0 ){
		int n = (_capacity<capacity)? _capacity : capacity;
		memcpy(items,items0,sizeof(void*)*n);
		delete [] items0;
	}
	capacity = _capacity;
}

