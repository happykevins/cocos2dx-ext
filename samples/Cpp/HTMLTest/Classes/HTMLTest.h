#ifndef __HTMLTEST_H__
#define __HTMLTEST_H__

#include "cocos2d.h"
#include <renren-ext.h>

USING_NS_CC;
USING_NS_CC_EXT;

class HTMLTest : public cocos2d::CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);

	bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
	void ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent);

	// HTML events
	void onHTMLClicked(
		IRichNode* root, IRichElement* ele, int _id);
	void onHTMLMoved(
		IRichNode* root, IRichElement* ele, int _id,
		const CCPoint& location, const CCPoint& delta);
    
    // implement the "static node()" method manually
    CREATE_FUNC(HTMLTest);
};

#endif // __HTMLTEST_H__
