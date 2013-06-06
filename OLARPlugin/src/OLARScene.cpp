#include "OLARPlugin.h"
#include "ARSub/ARSub.h"

USING_NS_CC;

OLARScene::OLARScene() : CCScene(){
	setAnchorPoint(ccp(0.5f, 0.5f));
	detectedTarget_ = NULL;
	listeners_ = new OLARArray();
	targets_ = new OLARArray();
}

OLARScene::~OLARScene(){
	for(int i=0;i<listeners_->Count();i++){
		OLARSceneEventListener *lsnr = (OLARSceneEventListener*)listeners_->ItemAt(i);
		if( lsnr ){ lsnr->release(); }
	}
	delete listeners_;
	for(int i=0;i<targets_->Count();i++){
		OLARTarget *target = (OLARTarget*)targets_->ItemAt(i);
		if( target ){ target->release(); }
	}
	delete targets_;
}

bool OLARScene::init()
{
    bool bRet = false;
	do{
		CCDirector * pDirector;
		CC_BREAK_IF( ! (pDirector = CCDirector::sharedDirector()) );
		this->setContentSize(pDirector->getWinSize());
		bRet = true;
	} while (0);
	CCDirector::sharedDirector()->setDepthTest(false);
	return bRet;
}

void OLARScene::visit(){
	OLARPlugin *plugin = OLARPlugin::SharedInstance();
	if( !plugin ){
		return;
	}
	if( plugin->IsNeedToRestart() ){
		plugin->Stop();
		plugin->Start();
	}
	
	kmGLMatrixMode(KM_GL_PROJECTION);
	kmGLPushMatrix();
	kmGLMatrixMode(KM_GL_MODELVIEW);
	kmGLPushMatrix();
	
	if( plugin->IsRunning() && ++frameCount_ > 5 ){
		bool prevVisiblity = isVisible();
		if( targets_->Count() ){
			OLARResult *result = NULL;
			OLARTarget *target = NULL;
			for(int i=0;i<targets_->Count();i++){
				OLARTarget *t = (OLARTarget*)targets_->ItemAt(i);
				if( !t ){ continue; }
				OLARResult *r = t->LastResult();
				if( r && ( !result || r->count > result->count) ){
					result = r;
					target = t;
				}
			}
			if( !result ){
				detectedTarget_ = NULL;
				setVisible(false);
			}else{
				kmMat4 modelview;
				kmMat4 projection;
				
				detectedTarget_ = target;
				
				setVisible(true);
				
				for(int i=0;i<16;i++){
					modelview.mat[i] = result->modelViewMatrix[i];
				}
				float farClip = target->Width()*16.f;
				plugin->GetProjectionMatrix(projection.mat,farClip);
				kmGLMatrixMode(KM_GL_PROJECTION);
				kmGLLoadMatrix( &projection );
				kmGLMatrixMode(KM_GL_MODELVIEW);
				kmGLLoadMatrix( &modelview );
				
				QMRect marker(-0.5*target->Width(),-0.5*target->Height(),target->Width(),target->Height());
				QMSize windowSize(getContentSize().width,getContentSize().height);
				kmGLTranslatef(-0.5*getContentSize().width,-0.5*getContentSize().height,0.0);
			}
		}
		plugin->RenderCameraImage();
		
		// Dispatch scene events for all listeners
		int lsnrCount = listeners_->Count();
		OLARSceneEventListener **lsnrs = (OLARSceneEventListener**)listeners_->Items();
		if( targets_ && prevVisiblity!=isVisible() ){
			if( isVisible() ){
				for(int i=0;i<lsnrCount;i++){
					if( lsnrs[i] ){ lsnrs[i]->OnVisible(this); }
				}
			}else{
				for(int i=0;i<lsnrCount;i++){
					if( lsnrs[i] ){ lsnrs[i]->OnInvisible(this); }
				}
			}
		}
		for(int i=0;i<lsnrCount;i++){
			if( lsnrs[i] ){ lsnrs[i]->OnUpdate(this); }
		}
	}
	
	CCScene::visit();
	
	kmGLMatrixMode(KM_GL_PROJECTION);
	kmGLPopMatrix();
	kmGLMatrixMode(KM_GL_MODELVIEW);
	kmGLPopMatrix();
}

void OLARScene::onEnter(){
	CCScene::onEnter();
	frameCount_ = 0;
	detectedTarget_ = NULL;
	setVisible(false);
	for(int i=0;i<targets_->Count();i++){
		OLARTarget *target = (OLARTarget*)targets_->ItemAt(i);
		if( target ){
			OLARPlugin::SharedInstance()->AddTarget(target);
		}
	}
}

void OLARScene::onEnterTransitionDidFinish(){
	CCScene::onEnterTransitionDidFinish();
	OLARPlugin::SharedInstance()->Start();
}

void OLARScene::onExit(){
	detectedTarget_ = NULL;
	CCScene::onExit();
	OLARPlugin::SharedInstance()->Stop();
	OLARPlugin::SharedInstance()->RemoveAllTargets();
}

OLARTarget* OLARScene::TargetAtIndex(int index){
	return (OLARTarget*)targets_->ItemAt(index);
}

void OLARScene::AddTarget(OLARTarget *target){
	if( !target ){ return; }
	target->retain();
	targets_->Add(target);
	if( !target->IsSizeSpecified() ){
		float size = cocos2d::CCDirector::sharedDirector()->getWinSizeInPixels().width;
		if( size < cocos2d::CCDirector::sharedDirector()->getWinSizeInPixels().height ) {
			size = cocos2d::CCDirector::sharedDirector()->getWinSizeInPixels().height;
		}
		target->SetSize(size);
	}
	if( isRunning() ){
		OLARPlugin::SharedInstance()->AddTarget(target);
	}
}

void OLARScene::RemoveTarget(OLARTarget *target){
	if( !target ){ return; }
	if( target==detectedTarget_ ){
		detectedTarget_ = NULL;
	}
	OLARPlugin::SharedInstance()->RemoveTarget(target);
	if( targets_->Remove(target) ){
		target->release();
	}
}

int OLARScene::NumberOfTargets(){
	return targets_->Count();
}

OLARTarget* OLARScene::DetectedTarget(){
	return detectedTarget_;
}

void OLARScene::AddEventListener(OLARSceneEventListener *listener){
	if( listener ){
		if( listeners_->Add(listener) ){
			listener->retain();
		}
	}
}

void OLARScene::RemoveEventListener(OLARSceneEventListener *listener){
	if( listener ){
		if( listeners_->Remove(listener) ){
			listener->release();
		}
	}
}

OLARScene *OLARScene::create(){
    OLARScene *pRet = new OLARScene();
    if (pRet && pRet->init()){
        pRet->autorelease();
        return pRet;
    }else{
        CC_SAFE_DELETE(pRet);
        return NULL;
    }
}

OLARScene *OLARScene::createWithTarget(OLARTarget *target){
	OLARScene *scene = OLARScene::create();
	if( scene ){ scene->AddTarget(target); }
	return scene;
}
