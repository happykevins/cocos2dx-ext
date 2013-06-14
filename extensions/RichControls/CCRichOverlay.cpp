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
#include "CCRichOverlay.h"
#include "CCRichElement.h"

NS_CC_EXT_BEGIN;

CCRichOverlay* CCRichOverlay::create()
{
	CCRichOverlay* overlay = new CCRichOverlay();
	if ( overlay && overlay->init() )
	{
		overlay->autorelease();
		return overlay;
	}

	CC_SAFE_DELETE(overlay);
	return NULL;
}

bool CCRichOverlay::init()
{
	//CCLayer::init();
	this->setTouchMode(kCCTouchesOneByOne);
	return true;
}

void CCRichOverlay::draw()
{
	CCLayer::draw();

#if CCRICH_DEBUG
	// debug bound box 
	const CCSize& s = this->getContentSize();
	CCPoint vertices[4]={
		ccp(0,0),ccp(s.width,0),
		ccp(s.width,s.height),ccp(0,s.height),
	};
	ccDrawColor4B(0x00, 0xff, 0x00, 0xff);
	ccDrawPoly(vertices, 4, true);
#endif
}

void CCRichOverlay::append(IRichElement* ele)
{
	REleHTMLTouchable* overlay = dynamic_cast<REleHTMLTouchable*>(ele);
	CCAssert(overlay, "[CCRich] not a overlay or subclass!");

	if ( overlay )
	{
		m_touchables.push_back(overlay);
	}
}

void CCRichOverlay::reset()
{
	removeAllChildren();
	m_elements.clear();
	m_touchables.clear();
	m_touched = NULL;
}

IRichNode* CCRichOverlay::getContainer()
{
	CCAssert(getParent(), "");
	return dynamic_cast<IRichNode*>(getParent());
}

bool CCRichOverlay::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
	// check is inside the rich node 
	RRect rect = getContainer()->getCompositor()->getRect();

	CCRect bbox;
	bbox.size = CCSize(rect.size.w, rect.size.h);
	bbox.origin = ccp(rect.pos.x, rect.pos.y - rect.size.h);

	CCPoint pt = convertToNodeSpace(pTouch->getLocation());

	if ( !bbox.containsPoint(convertToNodeSpace(pTouch->getLocation())) )
	{
		return false;
	}

	for( std::list<REleHTMLTouchable*>::iterator it = m_touchables.begin(); it != m_touchables.end(); it++ )
	{
		REleHTMLTouchable* overlay = *it;

		if ( overlay->isEnabled() && 
			overlay->isLocationInside(pt)
			/*overlay->onTouchBegan(this, pTouch, pEvent)*/)
		{
			//CCLog("[Rich Touch Began] at: %.0f, %.0f", pt.x, pt.y);
			m_touched = overlay;
			return true;
		}
	}

	return false;
}

void CCRichOverlay::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
{
	if ( m_touched && !m_eventhandlers.empty() )
	{
		CCPoint pt = convertToNodeSpace(pTouch->getLocation());
		//CCLog("[Rich Touch Moved] at: %.0f, %.0f", pt.x, pt.y);

		std::map<void*, IRichEventHandler*>::iterator hit = m_eventhandlers.begin();
		for ( ; hit != m_eventhandlers.end(); hit++ )
		{
			hit->second->onMoved(
				getContainer(), m_touched, m_touched->getID(), 
				pTouch->getLocation(), pTouch->getDelta());
		}

		//m_touched->onTouchMoved(this, pTouch, pEvent);
	}	
}

void CCRichOverlay::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
{
	if ( m_touched )
	{
		CCPoint pt = convertToNodeSpace(pTouch->getLocation());
		//CCLog("[Rich Touch Ended] at: %.0f, %.0f", pt.x, pt.y);

		if ( m_touched->isLocationInside(pt) )
		{
			std::map<void*, IRichEventHandler*>::iterator hit = m_eventhandlers.begin();
			for ( ; hit != m_eventhandlers.end(); hit++ )
			{
				hit->second->onClick(getContainer(), m_touched, m_touched->getID());
			}
			
		}
		
		//m_touched->onTouchEnded(this, pTouch, pEvent);
		m_touched = NULL;
	}	
}

void CCRichOverlay::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent)
{
	if ( m_touched )
	{
		CCPoint pt = convertToNodeSpace(pTouch->getLocation());
		//CCLog("[Rich Touch Cancelled] at: %.0f, %.0f", pt.x, pt.y);
		//m_touched->onTouchCancelled(this, pTouch, pEvent);
		m_touched = NULL;
	}	
}

void CCRichOverlay::registerListener(void* target, IRichEventHandler* listener)
{
	CCAssert(m_eventhandlers.find(target) == m_eventhandlers.end(), "dummy target! memory leak!" );

	m_eventhandlers.insert(std::make_pair(target, listener));
}

void CCRichOverlay::removeListener(void* target)
{
	std::map<void*, IRichEventHandler*>::iterator it = m_eventhandlers.find(target);

	if ( it != m_eventhandlers.end() )
	{
		delete it->second;
		m_eventhandlers.erase(it);
	}
}

CCRichOverlay::CCRichOverlay()
	: m_touched(NULL)
{
}

CCRichOverlay::~CCRichOverlay()
{
	std::map<void*, IRichEventHandler*>::iterator hit = m_eventhandlers.begin();
	for ( ; hit != m_eventhandlers.end(); hit++ )
	{
		delete hit->second;
	}
	m_eventhandlers.clear();

	m_container = NULL;
	m_touched = NULL;
}


NS_CC_EXT_END;


