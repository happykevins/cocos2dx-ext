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
#include "CCRichCompositor.h"

NS_CC_EXT_BEGIN;

bool RSimpleHTMLCompositor::composit(IRichElement* root)
{
	root->composit(this);

	RRect rect = getMetricsState()->elements_cache->flush(this);
	m_rRect.extend(rect);

	if ( m_rFontCache )
	{
		m_rFontCache->flush();
	}

	return true;
}

RSimpleHTMLCompositor::RSimpleHTMLCompositor(IRichNode* container)
	: RBaseCompositor(container)
{
	getRootCache()->setWrapline(true);
}

RMetricsState* RBaseCompositor::getMetricsState()
{
	if ( !m_rMetricsStack.empty() )
	{
		return &m_rMetricsStack.top();
	}

	return &m_rInitMetricsState;
}

RMetricsState* RBaseCompositor::pushMetricsState()
{
	m_rMetricsStack.push(RMetricsState(*getMetricsState()));
	return getMetricsState();
}

void RBaseCompositor::popMetricsState()
{
	if ( !m_rMetricsStack.empty() )
	{
		m_rMetricsStack.pop();
	}
}

RMetricsState* RBaseCompositor::initMetricsState(const RMetricsState* new_init_state /*= NULL*/)
{
	if ( new_init_state )
	{
		m_rInitMetricsState = *new_init_state;
	}
	while(!m_rMetricsStack.empty())
		m_rMetricsStack.pop();

	m_rInitMetricsState.elements_cache = getRootCache();
	m_rInitMetricsState.pen_x = 0;
	m_rInitMetricsState.pen_y = 0;

	return &m_rInitMetricsState;
}

RRenderState* RBaseCompositor::getRenderState()
{
	if ( !m_rRenderStack.empty() )
	{
		return &m_rRenderStack.top();
	}

	return &m_rInitRenderState;
}

RRenderState* RBaseCompositor::pushRenderState()
{
	m_rRenderStack.push(RRenderState(*getRenderState()));
	return getRenderState();
}

void RBaseCompositor::popRenderState()
{
	if ( !m_rRenderStack.empty() )
	{
		m_rRenderStack.pop();
	}
}

RRenderState* RBaseCompositor::initRenderState(const RRenderState* new_init_state/* = NULL*/)
{
	if ( new_init_state )
	{
		m_rInitRenderState = *new_init_state;
	}
	while(!m_rRenderStack.empty())
		m_rRenderStack.pop();

	return &m_rInitRenderState;
}

class dfont::FontCatalog* RBaseCompositor::getFont()
{
	using namespace dfont;

	if ( m_rFontCacheAlias 
		&& strcmp(m_rFontCacheAlias, getRenderState()->font_alias ) == 0 )
	{
		return m_rFontCache;
	}

	if (m_rFontCache)
		m_rFontCache->flush();

	m_rFontCacheAlias = getRenderState()->font_alias;
	m_rFontCache = FontFactory::instance()->find_font(m_rFontCacheAlias);

	return m_rFontCache;
}

// reset all state & cached data
void RBaseCompositor::reset()
{
	initMetricsState();
	initRenderState();

	getMetricsState()->zone = RRect();
	getMetricsState()->zone.size = getContainer()->getPreferredSize();

	getRootCache()->clear();

	m_rRect = RRect();
	m_rFontCacheAlias = NULL;
	m_rFontCache = NULL;
}

class IRichNode* RBaseCompositor::getContainer()
{
	return m_rContainer;
}

RBaseCompositor::RBaseCompositor(IRichNode* container)
	: m_rContainer(container),  m_rFontCache(NULL), m_rFontCacheAlias(NULL)
{
}

RBaseCompositor::~RBaseCompositor()
{
	//reset();
	m_rContainer = NULL;
}

NS_CC_EXT_END;

