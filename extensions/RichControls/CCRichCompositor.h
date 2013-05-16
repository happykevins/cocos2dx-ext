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
#ifndef __CC_RICHCOMPOSITOR_H__
#define __CC_RICHCOMPOSITOR_H__

#include "CCRichProtocols.h"
#include "CCRichCache.h"

#include <stack>

NS_CC_EXT_BEGIN;

class RBaseCompositor : public IRichCompositor
{
public:
	// get rect
	virtual const RRect& getRect() const { return m_rRect; }
	// get current composit state
	virtual RMetricsState* getMetricsState();
	// return new top state
	virtual RMetricsState* pushMetricsState();
	// return popped state
	virtual void popMetricsState();
	// initialize state
	virtual RMetricsState* initMetricsState(const RMetricsState* new_init_state = NULL);

	// get current composit state
	virtual RRenderState* getRenderState();
	// return new top state
	virtual RRenderState* pushRenderState();
	// return popped state
	virtual void popRenderState();
	// initialize state
	virtual RRenderState* initRenderState(const RRenderState* new_init_state = NULL);
	// get current cached font
	virtual class dfont::FontCatalog* getFont();

	// reset all state & cached data
	virtual void reset();
	virtual class IRichNode* getContainer();

	RBaseCompositor(IRichNode* container);
	virtual ~RBaseCompositor();

protected:
	IRichNode* m_rContainer;

	RMetricsState m_rInitMetricsState;
	std::stack<RMetricsState> m_rMetricsStack;
	RRenderState m_rInitRenderState;
	std::stack<RRenderState> m_rRenderStack;

	RRect m_rRect;

	class dfont::FontCatalog* m_rFontCache;
	const char* m_rFontCacheAlias;
};

//
// Simple Compositor for tree structured HTML elements
//
class RSimpleHTMLCompositor : public RBaseCompositor
{
public:
	virtual bool composit(IRichElement* root);
	virtual class ICompositCache* getRootCache() { return &m_rLineCache; }

	RSimpleHTMLCompositor(IRichNode* container);

private:
	RLineCache m_rLineCache;
};

NS_CC_EXT_END;

#endif//__CC_RICHCOMPOSITOR_H__
