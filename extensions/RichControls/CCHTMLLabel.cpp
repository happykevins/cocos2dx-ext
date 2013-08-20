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
#include "CCHTMLLabel.h"
#include "CCRichParser.h"
#include "CCRichCompositor.h"
#include "CCRichOverlay.h"

NS_CC_EXT_BEGIN;

//////////////////////////////////////////////////////////////////////////
/*

*/
//////////////////////////////////////////////////////////////////////////

CCHTMLLabel* CCHTMLLabel::create()
{
	CCHTMLLabel* label = new CCHTMLLabel;
	if ( label && label->initWithString(NULL, CCSize(.0f, .0f)) )
	{
		label->autorelease();
		return label;
	}

	CC_SAFE_DELETE(label);

	return NULL;
}

CCHTMLLabel* CCHTMLLabel::createWithString(const char* utf8_str, const CCSize& preferred_size, const char* font_alias)
{
	CCHTMLLabel* label = new CCHTMLLabel;
	if ( label && label->initWithString(utf8_str, preferred_size, font_alias) )
	{
		label->autorelease();
		return label;
	}

	CC_SAFE_DELETE(label);

	return NULL;
}

bool CCHTMLLabel::initWithString(const char* utf8_str, const CCSize& preferred_size, const char* font_alias)
{
	CCAssert(m_rRichNode == NULL, "");

	m_rRichNode = CCHTMLNode::createWithContainer(this);
	if( m_rRichNode == NULL )
		return false;

	m_rRichNode->retain();
	this->addChild(m_rRichNode);

	m_rRichNode->ignoreAnchorPointForPosition(true);
	m_rRichNode->setAnchorPoint(ccp(.0f, .0f));

	m_rRichNode->setDefaultWrapline(true);
	m_rRichNode->setDefaultSpacing(1);
	m_rRichNode->setDefaultPadding(0);
	m_rRichNode->setDefaultAlignment(e_align_left);
	m_rRichNode->setDefaultFontAlias(font_alias);
	m_rRichNode->setPreferredSize(RSize((short)preferred_size.width, (short)preferred_size.height));

	if ( utf8_str )
	{
		setString(utf8_str);
	}

	return true;
}

// from CCLabelProtocol
void CCHTMLLabel::setString(const char *utf8_str) 
{
	m_rRichNode->setStringUTF8(utf8_str);
}

const char* CCHTMLLabel::getString(void) 
{ 
	return m_rRichNode->getStringUTF8();
}

void CCHTMLLabel::appendString(const char *utf8_str)
{
	m_rRichNode->appendStringUTF8(utf8_str);
}

void CCHTMLLabel::draw()
{
	CCNode::draw();

#if CCRICH_DEBUG
	// draw bounding box
	const CCSize& s = this->getContentSize();
	CCPoint vertices[4]={
		ccp(0-1,0-1),ccp(s.width+1,0-1),
		ccp(s.width+1,s.height+1),ccp(0-1,s.height+1),
	};
	ccDrawColor4B(0xff, 0xff, 0xff, 0xff);
	ccDrawPoly(vertices, 4, true);
#endif
}

void CCHTMLLabel::registerListener(void* target, IRichEventHandler* listener)
{
	m_rRichNode->getOverlay()->registerListener(target, listener);
}

void CCHTMLLabel::removeListener(void* target)
{
	m_rRichNode->getOverlay()->removeListener(target);
}

CCHTMLLabel::CCHTMLLabel()
	: m_rRichNode(NULL)
{
}

CCHTMLLabel::~CCHTMLLabel()
{
	CC_SAFE_RELEASE(m_rRichNode);
}

NS_CC_EXT_END;

