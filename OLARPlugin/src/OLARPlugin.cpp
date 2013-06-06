#include "OLARPlugin.h"
#include "ARSub/ARSub.h"

USING_NS_CC;

struct OLARPluginData{
	OLAREngine *engine;
	ARCamera   *camera;
	int			ccProjection;
	bool		initialized;
	uint32_t	textureIds[2];
	float		texcoord[8];
	float		projRatio[2];
	int	        viewOrientation;
	bool		isDetected;
	float		lastMVP[3][4];
	bool		started;//
	bool		needRestart;
	jobject		activity;
	OLARTouchDelegate *touchDelegate;
};

static OLARPlugin __sharedInstance;

static void OLARPluginCaptureCallback(void *data, void *args){
	((OLARPlugin*)args)->CaptureCallback(data);
}

void* OLARPluginLoadBytesCallback(const char *path,int *length){
	return __sharedInstance.LoadDataFromFile(path,length);
}

OLARPlugin* OLARPlugin::SharedInstance(){
	return &__sharedInstance;
}

OLARPlugin::OLARPlugin(){
	_data = NULL;
	if( this!=&__sharedInstance ){ return; }
	_data = AR_ALLOC(OLARPluginData);
	memset(_data, 0, sizeof(OLARPluginData));
	OLAREngine::SetLoadBytesCallback(OLARPluginLoadBytesCallback);
}

OLARPlugin::~OLARPlugin(){
	AR_RELEASE(_data,OLARPluginData);
}

void OLARPlugin::Initialize(){
	if( !this || !_data ){ return; }
	if( _data->initialized ){ return; }
	ARConfig config = ARDefaultConfig;
	config.sizePreset = ARCameraSize640x480;
	config.pixelFormat = ARCameraFormatYUVSP;
	config.frameRate = 30;
	ARCamera *camera = ARCamera::GetDefaultCamera(false);
	if( !camera ){
		return;
	}
	_data->initialized = true;
	_data->camera = camera;
	camera->SetConfig(&config);
	camera->SetCaptureCallback(OLARPluginCaptureCallback,this);
	OLARDUMP("OLARPlugin::Initailize(): camera=%dx%d",camera->Width(),camera->Height());
}

void OLARPlugin::Release(){
	if( !this || !_data ){ return; }
	if( !_data->initialized ){ return; }
	OLARDUMP("OLARPlugin::Release()");
	SetPluginActivity(NULL);
	Stop();
	ARCamera::ReleseDefaultCamera();
	memset(_data, 0, sizeof(OLARPluginData));
}

bool OLARPlugin::IsInitialized(){
	return _data->initialized;
}

bool OLARPlugin::Start(){
	if( !this || !_data ){ return false; }
	if( !_data->initialized ){ Initialize(); }
	ARCamera *camera = _data->camera;
	if( !_data->camera || camera->IsRunning() ){ return false; }
	_data->started = true;
	_data->needRestart = false;
	
	OLARDUMP("Start OLARPlugin");
	
	ARGL::InitializeRenderer();
	
	glGenTextures(2,_data->textureIds);
	
	camera->SetTextures(_data->textureIds[0],_data->textureIds[1]);
	
	_data->viewOrientation = (int)GetDeviceOrientation();
	
	_data->touchDelegate = AR_ALLOC(OLARTouchDelegate);
	_data->touchDelegate->autorelease();
	CCDirector::sharedDirector()->getTouchDispatcher()
	->addTargetedDelegate(_data->touchDelegate,-(1<<16),false);
	
	float cw = camera->Width();
	float ch = camera->Height();
	float dw = CCDirector::sharedDirector()->getWinSize().width;
	float dh = CCDirector::sharedDirector()->getWinSize().height;
	float tx = 1.f;
	float ty = 1.f;
	
	if( dw > dh ){
		ty = (dw*ch)/(dh*cw);
		if( ty < 1.f ){
			ty = 1.f;
			tx = (dh*cw)/(dw*ch);
		}
	}else{
		float tmp = dw;
		dw = dh;
		dh = tmp;
		ty = (dw*ch)/(dh*cw);
		if( ty < 1.f ){
			ty = 1.f;
			tx = (dh*cw)/(dw*ch);
		}
	}
	
	float tx0 = (tx-1.f)*0.5f/tx;
	float tx1 = 1.f - tx0;
	float ty0 = (ty-1.f)*0.5f/ty;
	float ty1 = 1.f - ty0;
	float *texcoord = _data->texcoord;
	texcoord[0] = tx1;
	texcoord[1] = ty1;
	texcoord[2] = tx1;
	texcoord[3] = ty0;
	texcoord[4] = tx0;
	texcoord[5] = ty0;
	texcoord[6] = tx0;
	texcoord[7] = ty1;
	
	_data->projRatio[0] = tx;
	_data->projRatio[1] = ty;
	
	OLARDUMP("cw=%f,ch=%f / dw=%f,dh=%f / rtx=%f,rty=%f",cw,ch,dw,dh,tx,ty);
	OLARDUMP("ty = %f -> %f",ty0,ty1);
	
	return camera->Start();
	
}


bool OLARPlugin::Stop(){
	if( !this || !_data ){ return false; }
	if(_data->touchDelegate){
		CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(_data->touchDelegate);
		_data->touchDelegate = NULL;
	}
	if( _data->started ){
		_data->started=false;
		OLARDUMP("Stop OLARPlugin");
		glDeleteTextures(2,_data->textureIds);
		ARGL::FinalizeRenderer();
	}
	ARCamera *camera = _data->camera;
	if( !camera || !camera->IsRunning() ){ return false; }
	return camera->Stop();
}

bool OLARPlugin::IsStarted(){
	if( !this || !_data ){ return false; }
	return _data->started;
}

bool OLARPlugin::IsNeedToRestart(){
	if( !this || !_data ){ return false; }
	return _data->needRestart;
}

void OLARPlugin::SetNeedRestart(bool v){
	if( !this || !_data ){ return; }
	_data->needRestart = v;
}

void OLARPlugin::AddTarget(OLARTarget *target){
	if( !this || !_data ){ return; }
	if( !_data->initialized ){ Initialize(); }
	if( !_data->engine ){
		int w = _data->camera->Width();
		int h = _data->camera->Height();
		int bpp = _data->camera->Bpp();
		_data->engine = AR_ALLOC(OLAREngine);
		_data->engine->Initialize(w,h,bpp);
	}
	if( !target->Engine() ){
		_data->engine->AddTarget(target);
	}
}

void OLARPlugin::RemoveTarget(OLARTarget *target){
	if( !this || !_data ){ return; }
	if( !_data->engine ){ return; }
	_data->engine->RemoveTarget(target);
	if( _data->engine->NumberOfTargets()==0 ){
		AR_RELEASE(_data->engine,OLAREngine);
	}
}

void OLARPlugin::RemoveAllTargets(){
	if( !this || !_data ){ return; }
	if( !_data->engine ){ return; }
	_data->engine->RemoveAllTargets();
	AR_RELEASE(_data->engine,OLAREngine);
}

void OLARPlugin::CaptureCallback(void *data){
	if( !this || !_data ){ return; }
	if( _data->engine ){
		_data->engine->Track(data);
		UpdateLastMVP(_data->engine->LastResult());
	}
}

void OLARPlugin::RenderCameraImage(){
	if( !this || !_data ){ return; }
	if( !_data->camera ){ return; }
	ARCamera *camera = _data->camera;
	camera->UpdateTexture();
	float t[8];
	int dir = 3;
	int vo = _data->viewOrientation;
	if( vo == AROrientationLandscapeLeft ){
		dir = 1;
	}else if( vo == AROrientationPortrait ){
		dir = 0;
	}else if( vo == AROrientationPortraitUpsideDown ){
		dir = 2;
	}
	for(int i=0;i<4;i++){
		int j = 2*((i+dir)%4);
		t[2*i+0] = _data->texcoord[j];
		t[2*i+1] = _data->texcoord[j+1];
	}
	ARGL::RenderYUVImage(_data->textureIds[0], _data->textureIds[1], t);
}

bool OLARPlugin::GetCameraImage(OLARImage *image)
{
	if( !this || !_data || !image ){ return false; }
	ARCamera *camera = _data->camera;
	if( !camera || !camera->PixelBuffer(0) ){
		return false;
	}
	camera->LockBuffer();
	image->width = camera->Width();
	image->height = camera->Height();
	image->bpp = camera->Bpp();
	image->data = (uint8_t*)camera->PixelBuffer(0);
	if( image->bpp == 1 ){
		image->extdata = (uint8_t*)camera->PixelBuffer(1);
	}else{
		image->extdata = NULL;
	}
	camera->UnlockBuffer();
	return true;
}

bool OLARPlugin::IsRunning(){
	if( !this || !_data ){ return false; }
	return (_data->camera && _data->camera->IsRunning());
}

void OLARPlugin::GetProjectionMatrix(float *proj,float farClip){
	if( !this || !_data ){ return; }
	if( _data->engine ){
		float *p = _data->engine->ProjectionMatrix(farClip);
		for(int i=0;i<16;i++){
			proj[i] = p[i];
		}
		proj[0] *= _data->projRatio[0];
		proj[5] *= _data->projRatio[1];
		int vo = _data->viewOrientation;
		if( vo == AROrientationLandscapeLeft ){
			proj[0] *= -1.f;
			proj[5] *= -1.f;
		}else if( vo == AROrientationPortrait ||
				 vo == AROrientationPortraitUpsideDown ){
			proj[1] = -proj[0];
			proj[4] =  proj[5];
			proj[5] = proj[0] = 0.f;
			if( vo == AROrientationPortraitUpsideDown ){
				proj[1] *= -1.f;
				proj[4] *= -1.f;
			}
		}
	}
}

void OLARPlugin::UpdateLastMVP(OLARResult *result){
	if( !this || !_data ){ return; }
	if( !result || !_data->engine ){
		_data->isDetected = false;
		return;
	}
	_data->isDetected = true;
	float *mv = result->modelViewMatrix;
	float R[3][4];
	for(int i=0;i<3;i++){
		for(int j=0;j<4;j++){
			R[i][j] = mv[j*4+i];
		}
	}
	float L[3][4];
	float (*lastMVP)[4] = _data->lastMVP;
	_data->engine->GetCameraMatrix(L);
	for(int i=0;i<3;i++){
		lastMVP[i][0] = L[i][0] * R[0][0] + L[i][1] * R[1][0] + L[i][2] * R[2][0];
		lastMVP[i][1] = L[i][0] * R[0][1] + L[i][1] * R[1][1] + L[i][2] * R[2][1];
		lastMVP[i][2] = L[i][0] * R[0][2] + L[i][1] * R[1][2] + L[i][2] * R[2][2];
		lastMVP[i][3] = L[i][0] * R[0][3] + L[i][1] * R[1][3] + L[i][2] * R[2][3] + L[i][3];
	}
}

CCPoint OLARPlugin::ConvertCocosPosToCameraPos(CCPoint p){
	if( !this || !_data ){
		return CCPointMake(-1, -1);
	}
	float x0 = p.x;
	float y0 = p.y;
	float dw = CCDirector::sharedDirector()->getWinSize().width;
	float dh = CCDirector::sharedDirector()->getWinSize().height;
	
	float rx = x0;
	float ry = y0;
	int vo = _data->viewOrientation;
	if( vo == AROrientationPortrait){
		rx = y0;
		ry = dw - x0;
	}else if( vo == AROrientationLandscapeLeft){
		rx = dw - x0;
		ry = dh - y0;
	}else if( vo == AROrientationPortraitUpsideDown){
		rx = dh - y0;
		ry = x0;
	}
	
	x0 = rx;
	y0 = ry;
	
	if(dw<dh){
		float t = dw;
		dw = dh;
		dh = t;
	}
	ARCamera *camera = _data->camera;
	float *projRatio = _data->projRatio;
	x0 = (((x0/dw)-0.5f)/projRatio[0]+0.5f)*camera->Width();
	y0 = (((y0/dh)-0.5f)/projRatio[1]+0.5f)*camera->Height();
	
	return CCPointMake(x0,y0);
}

CCPoint OLARPlugin::GetLocationInTarget(float x0, float y0){
	if( !this || !_data || !_data->isDetected ){
		return CCPointMake(-1, -1);
	}
	float dw = CCDirector::sharedDirector()->getWinSize().width;
	float dh = CCDirector::sharedDirector()->getWinSize().height;
	
	CCPoint cameraPos = ConvertCocosPosToCameraPos(CCPointMake(x0,y0));
	
	x0 = cameraPos.x;
	y0 = cameraPos.y;
	
	float (*m)[4] = _data->lastMVP;
	float a = m[0][0]-x0*m[2][0];
	float b = m[0][1]-x0*m[2][1];
	float c = m[1][0]-y0*m[2][0];
	float d = m[1][1]-y0*m[2][1];
	float t = a*d - b*c;
	if( t == 0.f ){
		return CCPointMake(-1, -1);
	}
	float ex = m[2][3]*x0 - m[0][3];
	float ey = m[2][3]*y0 - m[1][3];
	float x1 = ( d*ex-b*ey )/t + dw*0.5f;
	float y1 = -(-c*ex+a*ey )/t + dh*0.5f;
	return CCPointMake(x1,y1);
}

int OLARPlugin::GetDeviceOrientation(){
	if( !this || !_data ){ return (int)AROrientationUnknown; }
#if ANDROID
	return ARGetDeviceOrientation(_data->activity);
#else
	return ARGetDeviceOrientation();
#endif
}

uint8_t* OLARPlugin::LoadDataFromFile(const char *path,int *length){
#if ANDROID
	if( !this || !_data || !_data->activity ){ return NULL; }
	return ARLoadDataFromFile(_data->activity,path,length);
#else
	return ARLoadDataFromFile(path,length);
#endif
}


void OLARPlugin::SetPluginActivity(void* activity){
#if ANDROID
	if( this==NULL || _data == NULL ){ return; }
	JNIEnv *env = getJNIEnv();
	if(env==NULL){
		OLARDUMP("%s:JNIEnv is null",__FUNCTION__);
		return;
	}
	if( _data->activity != NULL ){
		env->DeleteGlobalRef(_data->activity);
		_data->activity = NULL;
	}
	if( activity != NULL ){
		_data->activity = env->NewGlobalRef((jobject)activity);
	}
	if( _data->camera ){
		((ARAndroidCamera*)_data->camera)->SetPluginActivity((jobject)activity);
	}
#endif
}

void* OLARPlugin::GetPluginActivity(){
#if ANDROID
	if( this==NULL || _data == NULL ){ return NULL; }
	return _data->activity;
#else
	return NULL;
#endif
}

void OLARPlugin::SetCameraConfig(int width,int height,int pixelFormat,float fov){
	if( this==NULL || _data == NULL ){ return; }
	ARCamera *camera = _data->camera;
	if( camera ){
		camera->SetCaptureImageConfig(width,height,pixelFormat);
	}
}

bool OLARPlugin::IsSimulator(){
#if TARGET_IPHONE_SIMULATOR
	return true;
#else
	return false;
#endif
}

// static method
void OLARPlugin::SetSimulatorData(const char *path,int previewFPS){
#if TARGET_IPHONE_SIMULATOR
	OLARPlugin *plugin = OLARPlugin::SharedInstance();
	if( !plugin ){ return; }
	plugin->Initialize();
	if( !plugin->_data || !plugin->_data->camera ){ return; }
	plugin->_data->camera->SetFrameRate((float)previewFPS);
	plugin->_data->camera->SetSimulatorData(path);
#endif
}

////////////////////////////////////////////////
//             OLARTouchDelegate              //
////////////////////////////////////////////////

OLARTouchDelegate::OLARTouchDelegate() : CCTouchDelegate(), CCObject(){}

bool OLARTouchDelegate::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
	ConvertTouches(pTouch);
	return true;
}

void OLARTouchDelegate::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
	ConvertTouches(pTouch);
}

void OLARTouchDelegate::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
	ConvertTouches(pTouch);
}

void OLARTouchDelegate::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
	ConvertTouches(pTouch);
}

void OLARTouchDelegate::ConvertTouches(CCTouch *touch){
	CCPoint p = touch->getLocationInView();
	CCPoint p2 =  OLARPlugin::SharedInstance()->GetLocationInTarget(p.x,p.y);
	touch->setTouchInfo(touch->getID(),p2.x,p2.y);
}
