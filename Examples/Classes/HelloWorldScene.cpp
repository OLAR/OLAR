#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include <OLARPlugin.h>

using namespace cocos2d;
using namespace CocosDenshion;

class MySceneEventListener : public OLARSceneEventListener{
public:
	virtual void OnVisible(OLARScene* scene){
		OLARTarget *detectedTarget = scene->DetectedTarget();
		if( detectedTarget ){
			OLARDUMP("Target detected! (targetId=%d)",detectedTarget->TargetId());
		}
	}
	virtual void OnInvisible(OLARScene* scene){}
	virtual void OnUpdate(OLARScene* scene){}
};

CCScene* HelloWorld::scene()
{
	// You can also use movie file as a dummy camera.
	// This feature is only available on iOS simulator environment.
	if( OLARPlugin::IsSimulator() ){
		// Play dummy.mov as dummy camera with 10 fps
		OLARPlugin::SetSimulatorData("dummy.mov",10);
	}
	
	// Instantiate OLARScene for AR scene.
    OLARScene *scene = OLARScene::create();
	
    // Add layer as a child to scene.
    HelloWorld *layer = HelloWorld::create();
    scene->addChild(layer);
	
	// Add image data of recognition targets.
	// You can add multiple targets at the same time.
	scene->AddTarget(OLARTarget::TargetWithPath("img01.qdb"));
	scene->AddTarget(OLARTarget::TargetWithPath("img02.qdb"));
	
	// Add AR event listener that handles some AR events.
	// See interface declaration of OLARSceneEventListener.
	MySceneEventListener *listener = new MySceneEventListener();
	scene->AddEventListener(listener);
	listener->release();
    
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() ){
        return false;
    }
	
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback) );
    pCloseItem->setPosition( ccp(CCDirector::sharedDirector()->getWinSize().width - 20, 20) );

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition( CCPointZero );
    this->addChild(pMenu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    CCLabelTTF* pLabel = CCLabelTTF::create("Hello World", "Thonburi", 34);

    // ask director the window size
    CCSize size = CCDirector::sharedDirector()->getWinSize();

    // position the label on the center of the screen
    pLabel->setPosition( ccp(size.width / 2, size.height - 20) );

    // add the label as a child to this layer
    this->addChild(pLabel, 1);

    // add "HelloWorld" splash screen"
    CCSprite* pSprite = CCSprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    pSprite->setPosition( ccp(size.width/2, size.height/2) );

    // add the sprite as a child to this layer
    this->addChild(pSprite, 0);
    
    return true;
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
