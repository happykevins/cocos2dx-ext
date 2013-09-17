#include "HTMLTest.h"
#include "AppMacros.h"
#include <label_nodes/CCLabelBMFont.h>
#include <label_nodes/CCLabelTTF.h>
#include <textures/CCTexture2D.h>
#include <support/ccUTF8.h>
#include <platform/CCFileUtils.h>
#include <misc_nodes/CCClippingNode.h>

#include <cocos-ext.h>


static CCHTMLLabel* s_htmlLabel = NULL;
std::string tt;

CCScene* HTMLTest::scene()
{
	//cells_test();

    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HTMLTest *layer = HTMLTest::create();

    // add layer as a child to scene
    scene->addChild(layer);

	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(layer, 0, false);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HTMLTest::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }

	CCLayerColor* l = CCLayerColor::create(ccc4(0xb0, 0xb0, 0xb0, 0xff));
	l->setContentSize(this->getContentSize());
	this->addChild(l);
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "res/close.png",
                                        "res/close.png",
                                        this,
                                        menu_selector(HTMLTest::menuCloseCallback));
    
	pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width/2 ,
                                origin.y + pCloseItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);


	using namespace dfont;

	// font1
	FontCatalog* font_catalog = NULL;
	font_catalog = FontFactory::instance()->create_font(
		"font1", "simhei.ttf", 0xffffffff, 32, e_plain, 0.0f, 0xffffffff, 0);
	// font2
	font_catalog = FontFactory::instance()->create_font(
		"font2", "simkai.ttf", 0xffffffff, 24, e_shadow, 1.0f, 0xff000000, 0);
	font_catalog->add_hackfont("htmltest/Marker Felt.ttf", latin_charset(), -1);
	// font3
	font_catalog = FontFactory::instance()->create_font(
		"font3", "simli.ttf", 0xffffffff, 20, e_border, 1.0f, 0xff000000, 0);
	font_catalog->add_hackfont("simhei.ttf", latin_charset(), 5);


	//////////////////////////////////////////////////////////////////////////
	
	CCSize vsize = CCDirector::sharedDirector()->getVisibleSize();
	CCString* str_utf8 = CCString::createWithContentsOfFile("html.htm");

	CCHTMLLabel* htmllabel = CCHTMLLabel::createWithString(str_utf8->getCString(), 
		CCSize(vsize.width*0.8f, vsize.height), "default");
	htmllabel->setAnchorPoint(ccp(0.5f,0.5f));
	htmllabel->setPosition(ccp(vsize.width*0.5f, vsize.height*0.5f));

	addChild(htmllabel);

	s_htmlLabel = htmllabel;

	htmllabel->registerListener(this, &HTMLTest::onHTMLClicked, &HTMLTest::onHTMLMoved );

	FontFactory::instance()->dump_textures();

    return true;
}

void HTMLTest::onHTMLClicked(
	IRichNode* root, IRichElement* ele, int _id)
{
	CCLog("[On Clicked] id=%d", _id);

	if ( !s_htmlLabel )
	{
		return;
	}
	else if ( _id == 1002 ) // close
	{
		s_htmlLabel->setVisible(false);
	}
	else if ( _id == 2000 ) //reload
	{
		CCString* str_utf8 = CCString::createWithContentsOfFile("htmltest/html.htm");
		s_htmlLabel->setString(str_utf8->getCString());	
	}
}

void HTMLTest::onHTMLMoved(
	IRichNode* root, IRichElement* ele, int _id,
	const CCPoint& location, const CCPoint& delta)
{
	CCLog("[On Moved] id=%d", _id);

	if ( !s_htmlLabel )
	{
		return;
	}
	else if ( _id == 1001 )
	{
		s_htmlLabel->setPosition(ccpAdd(delta, s_htmlLabel->getPosition()));
	}
}


void HTMLTest::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool HTMLTest::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
	return true;
}

 void HTMLTest::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
 {
 }
