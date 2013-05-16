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
#ifndef __CC_HTMLLABEL_H__
#define __CC_HTMLLABEL_H__

#include "CCRichNode.h"

NS_CC_EXT_BEGIN;

#define CCNODE_UTILITY_SETTER(func_name, type_name) \
	virtual void func_name(type_name v) \
	{ m_rRichNode->func_name(v); } \

#define CCNODE_UTILITY_GETTER(func_name, type_name) \
	virtual type_name func_name() \
	{ return m_rRichNode->func_name(); } \

//
// HTML Label can Parse & Display Simple HTML Content Text
//
// Standard HTML Tags Supported:
//	- <p>		: style="white-space:nowrap; color; font; text-align:left|center|right; margin; padding; line-height(no use)"
//	- <br>		:
//	- <hr>		: width; size; style="color"
//	- <font>	: face; color
//	- <u>		: 
//	- <table>	: width; align; cellspadding; cellsspacing; border; bgcolor; bordercolor; frame; rules
//	- <tr>		: align; valign
//	- <td>		: width; height; align; valign; padding; spacing; nowrap
//	- <img>		: src; alt(no use); style="textrue-rect:<TOP>px <RIGHT>px <BOTTOM>px <LEFT>px"
//	- <button>	: name; value; bgcolor
//
//	Extension Supported:
//	- <ccb>		: src; play="auto"; anim;
//
class CCHTMLLabel : public CCNode, public CCLabelProtocol
{
public:
	static CCHTMLLabel* create();
	static CCHTMLLabel* createWithString(const char* rich_string, const CCSize& preferred_size, const char* font_alias = DFONT_DEFAULT_FONTALIAS);
	bool initWithString(const char* rich_string, const CCSize& preferred_size, const char* font_alias = DFONT_DEFAULT_FONTALIAS);

	// from CCLabelProtocol
	virtual void setString(const char *label);
	virtual const char* getString(void);

	// append string, faster if you only add additional string to tail
	virtual void appendString(const char *label);

	// from CCLayer
	virtual void draw();

	// utilities
	CCNODE_UTILITY_SETTER(setPreferredSize,			RSize);
	CCNODE_UTILITY_SETTER(setPlainMode,				bool);
	CCNODE_UTILITY_SETTER(setDefaultFontAlias,		const char*);
	CCNODE_UTILITY_SETTER(setDefaultColor,			unsigned int);
	CCNODE_UTILITY_SETTER(setDefaultAlignment,		RMetricsState::EAlign);
	CCNODE_UTILITY_SETTER(setDefaultNoWrapline,		bool);
	CCNODE_UTILITY_SETTER(setDefaultSpacing,		short);
	CCNODE_UTILITY_SETTER(setDefaultPadding,		short);

	CCNODE_UTILITY_GETTER(getPreferredSize,			RSize);
	CCNODE_UTILITY_GETTER(isPlainMode,				bool);
	CCNODE_UTILITY_GETTER(getDefaultFontAlias,		const char*);
	CCNODE_UTILITY_GETTER(getDefaultColor,			unsigned int);
	CCNODE_UTILITY_GETTER(getDefaultAlignment,		RMetricsState::EAlign);
	CCNODE_UTILITY_GETTER(isDefaultNoWrapline,		bool);
	CCNODE_UTILITY_GETTER(getDefaultSpacing,		short);
	CCNODE_UTILITY_GETTER(getDefaultPadding,		short);

	CCHTMLLabel();
	virtual ~CCHTMLLabel();

private:
	class CCRichNode* m_rRichNode;
};

NS_CC_EXT_END;

#endif//__CC_HTMLLABEL_H__
