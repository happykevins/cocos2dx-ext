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

#if CCRICH_ENABLE_LUA_BINDING
#	include "CCLuaEngine.h"
#	include "CCRichElement.h"
#endif

NS_CC_EXT_BEGIN;

// lua binding
#if CCRICH_ENABLE_LUA_BINDING
class REvLuaHandler : public IRichEventHandler
{
public:
	REvLuaHandler(int hclick, int hmoved) 
		: m_clickhandle(hclick), m_movedhandle(hmoved)
	{
	}

	virtual void onClick(IRichNode* root, IRichElement* ele, int _id)
	{
		if ( m_clickhandle )
		{
			REleHTMLTouchable* touchable = dynamic_cast<REleHTMLTouchable*>(ele);
			if ( CCLuaEngine::defaultEngine() && touchable )
			{
				CCLuaStack* stack = CCLuaEngine::defaultEngine()->getLuaStack();
				stack->pushInt(_id);
				stack->pushString(touchable->getName().c_str(), touchable->getName().size());
				stack->pushString(touchable->getValue().c_str(), touchable->getValue().size());
				stack->executeFunctionByHandler(m_clickhandle, 3);
			}
		}
	}

	virtual void onMoved(IRichNode* root, IRichElement* ele, int _id, const CCPoint& location, const CCPoint& delta)
	{
		if ( m_movedhandle )
		{
			REleHTMLTouchable* touchable = dynamic_cast<REleHTMLTouchable*>(ele);
			if ( CCLuaEngine::defaultEngine() && touchable )
			{
				CCLuaStack* stack = CCLuaEngine::defaultEngine()->getLuaStack();
				stack->pushInt(_id);
				stack->pushString(touchable->getName().c_str(), touchable->getName().size());
				stack->pushString(touchable->getValue().c_str(), touchable->getValue().size());
				stack->pushFloat(location.x);
				stack->pushFloat(location.y);
				stack->pushFloat(delta.x);
				stack->pushFloat(delta.y);
				stack->executeFunctionByHandler(m_movedhandle, 7);
			}
		}
	}

protected:
	int m_clickhandle;
	int m_movedhandle;
};
#endif//#if CCRICH_ENABLE_LUA_BINDING

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
//	- <p>		: id; style="white-space:nowrap; color; font; text-align:left|center|right; margin; padding; line-height(no use)"
//	- <br>		: id; 
//	- <hr>		: id; width; size; style="color"
//	- <font>	: id; face; color
//	- <u>		: id; 
//	- <table>	: id; width; align; cellspadding; cellsspacing; border; bgcolor; bordercolor; frame; rules
//	- <tr>		: id; align; valign
//	- <td>		: id; width; height; align; valign; padding; spacing; nowrap; bgcolor; bg-image; bg-rect
//	- <img>		: id; src; alt(no use); texture-rect="<TOP>px <LEFT>px <BOTTOM>px <LEFT>px"
//	- <a>		: id; name; href; bgcolor
//	- <button>	: id; name; value; bgcolor
//
//	Extension Supported:
//	- <ccb>		: id; src; play="auto"; anim;
//
class CCHTMLLabel : public CCNode, public CCLabelProtocol
{
public:
	static CCHTMLLabel* create();
	static CCHTMLLabel* createWithString(const char* utf8_str, const CCSize& preferred_size, const char* font_alias = DFONT_DEFAULT_FONTALIAS);
	bool initWithString(const char* utf8_str, const CCSize& preferred_size, const char* font_alias = DFONT_DEFAULT_FONTALIAS);

	// from CCLabelProtocol
	virtual void setString(const char *utf8_str);
	virtual const char* getString(void);

	// append string, faster if you only add additional string to tail
	virtual void appendString(const char *utf8_str);

	// from CCLayer
	virtual void draw();

	// event handler
	template<typename T>
	void registerClickListener(T* _target, typename REvHandler<T>::mfunc_click_t _f)
	{
		registerListener(_target, new REvHandler<T>(_target, _f));
	}
	template<typename T>
	void registerMovedListener(T* _target, typename REvHandler<T>::mfunc_moved_t _f)
	{
		registerListener(_target, new REvHandler<T>(_target, _f));
	}
	template<typename T>
	void registerListener(T* _target, typename REvHandler<T>::mfunc_click_t _cf, typename REvHandler<T>::mfunc_moved_t _mf)
	{
		registerListener(_target, new REvHandler<T>(_target, _cf, _mf));
	}

	void registerListener(void* target, IRichEventHandler* listener);
	void removeListener(void* target);

#if CCRICH_ENABLE_LUA_BINDING
	void registerLuaClickListener(int click_handle)
	{
		if (click_handle)
			registerListener((void*)click_handle, new REvLuaHandler(click_handle, 0));
	}
	void registerLuaMovedListener(int moved_handle)
	{
		if (moved_handle)
			registerListener((void*)moved_handle, new REvLuaHandler(0, moved_handle));
	}
	void removeLuaListener(int handle)
	{
		removeListener((void*)handle);
	}
#endif//CCRICH_ENABLE_LUA_BINDING


	// utilities
	CCNODE_UTILITY_SETTER(setPreferredSize,			RSize);
	CCNODE_UTILITY_SETTER(setPlainMode,				bool);
	CCNODE_UTILITY_SETTER(setDefaultFontAlias,		const char*);
	CCNODE_UTILITY_SETTER(setDefaultColor,			unsigned int);
	CCNODE_UTILITY_SETTER(setDefaultAlignment,		EAlignment);
	CCNODE_UTILITY_SETTER(setDefaultWrapline,		bool);
	CCNODE_UTILITY_SETTER(setDefaultSpacing,		short);
	CCNODE_UTILITY_SETTER(setDefaultPadding,		short);

	CCNODE_UTILITY_GETTER(getPreferredSize,			RSize);
	CCNODE_UTILITY_GETTER(isPlainMode,				bool);
	CCNODE_UTILITY_GETTER(getDefaultFontAlias,		const char*);
	CCNODE_UTILITY_GETTER(getDefaultColor,			unsigned int);
	CCNODE_UTILITY_GETTER(getDefaultAlignment,		EAlignment);
	CCNODE_UTILITY_GETTER(isDefaultWrapline,		bool);
	CCNODE_UTILITY_GETTER(getDefaultSpacing,		short);
	CCNODE_UTILITY_GETTER(getDefaultPadding,		short);


	CCHTMLLabel();
	virtual ~CCHTMLLabel();

private:
	class CCRichNode* m_rRichNode;
};

NS_CC_EXT_END;

#endif//__CC_HTMLLABEL_H__
