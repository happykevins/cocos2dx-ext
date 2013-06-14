/****************************************************************************
 Copyright (c) 2013 Kevin Sun and RenRen Games

 email:happykevins@gmail.com
 http://wan.renren.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#ifndef __CC_RICHOVERLAY_H__
#define __CC_RICHOVERLAY_H__

#include "CCRichProtocols.h"

NS_CC_EXT_BEGIN;

class CCRichOverlay : public CCLayer
{
public:
	static CCRichOverlay* create();

	virtual bool init();
	virtual void append(class IRichElement* ele);
	virtual void reset();

	// from CCLayer
	virtual void draw();
	virtual bool ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
	virtual void ccTouchMoved(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
	virtual void ccTouchEnded(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
	virtual void ccTouchCancelled(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);

	// register listener
	virtual void registerListener(void* target, IRichEventHandler* listener);
	virtual void removeListener(void* target);

	CCRichOverlay();
	virtual ~CCRichOverlay();

private:
	IRichNode* getContainer();

	std::list<class REleHTMLTouchable*> m_elements;
	std::list<class REleHTMLTouchable*> m_touchables;

	std::map<void*, IRichEventHandler*> m_eventhandlers;
	class REleHTMLTouchable* m_touched;
	class IRichNode* m_container;
};


NS_CC_EXT_END;


#endif//__CC_RICHOVERLAY_H__

