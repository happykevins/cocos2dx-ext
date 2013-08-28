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
#include "CCRichNode.h"
#include "CCRichAtlas.h"
#include "CCRichParser.h"
#include "CCRichCompositor.h"
#include "CCRichOverlay.h"

NS_CC_EXT_BEGIN;

IRichParser* CCRichNode::getParser()
{
	return m_rParser;
}

IRichCompositor* CCRichNode::getCompositor()
{
	return m_rCompositor;
}

RSize CCRichNode::getActualSize()
{
	return getCompositor()->getRect().size;
}

RSize CCRichNode::getPreferredSize()
{
	return m_rPreferedSize;
}

void CCRichNode::setPreferredSize(RSize size) 
{ 
	if ( size.w != m_rPreferedSize.w || size.h != m_rPreferedSize.h )
	{
		m_rPreferedSize = size; 
		updateAll();
	}
}

const char* CCRichNode::getDefaultFontAlias()
{
	return getCompositor()->getRenderState()->font_alias;
}

void CCRichNode::setDefaultFontAlias(const char* font_alias)
{
	if ( getCompositor()->getRenderState()->font_alias != font_alias )
	{
		getCompositor()->getRenderState()->font_alias = font_alias;
		updateAll();
	}
}



void CCRichNode::setDefaultColor(unsigned int color)
{
	if ( getCompositor()->getRenderState()->color != color )
	{
		getCompositor()->getRenderState()->color = color;
		updateAll();
	}
}

unsigned int CCRichNode::getDefaultColor()
{
	return getCompositor()->getRenderState()->color;
}

EAlignment CCRichNode::getDefaultAlignment()
{
	return getCompositor()->getRootCache()->getHAlign();
}

void CCRichNode::setDefaultAlignment(EAlignment align)
{
	if ( getCompositor()->getRootCache()->getHAlign() != align )
	{
		getCompositor()->getRootCache()->setHAlign(align);
		updateAll();
	}
}

bool CCRichNode::isDefaultWrapline()
{
	return getCompositor()->getRootCache()->isWrapline();
}

void CCRichNode::setDefaultWrapline(bool wrapline)
{
	if ( getCompositor()->getRootCache()->isWrapline() != wrapline )
	{
		getCompositor()->getRootCache()->setWrapline(wrapline);
		updateAll();
	}
}

short CCRichNode::getDefaultSpacing()
{
	return getCompositor()->getRootCache()->getSpacing();
}

void CCRichNode::setDefaultSpacing(short spacing)
{
	if ( getCompositor()->getRootCache()->getSpacing() != spacing )
	{
		getCompositor()->getRootCache()->setSpacing(spacing);
		updateAll();
	}
}

short CCRichNode::getDefaultPadding()
{
	return getCompositor()->getRootCache()->getPadding();
}

void CCRichNode::setDefaultPadding(short padding)
{
	if ( getCompositor()->getRootCache()->getPadding() != padding )
	{
		getCompositor()->getRootCache()->setPadding(padding);
		updateAll();
	}
}

void CCRichNode::setStringUTF8(const char* utf8_str)
{
	m_rRichString = utf8_str;
	updateAll();
}

void CCRichNode::appendStringUTF8(const char* utf8_str)
{
	m_rRichString.append(utf8_str);
	processRichString(utf8_str);
}

const char* CCRichNode::getStringUTF8()
{
	return m_rRichString.c_str();
}

IRichAtlas* CCRichNode::findAtlas(class CCTexture2D* texture, unsigned int color_rgba, int zorder /*= 0*/)
{
	return findColoredTextureAtlas(texture, color_rgba, zorder);
}

void CCRichNode::addOverlay(IRichElement* overlay)
{
	getOverlay()->append(overlay);
}

void CCRichNode::addCCNode(class CCNode* node)
{
	getOverlay()->addChild(node);
}

void CCRichNode::removeCCNode(class CCNode* node)
{
	getOverlay()->removeChild(node);
}

CCRichOverlay* CCRichNode::getOverlay()
{
	if ( !m_rOverlays )
	{
		m_rOverlays = CCRichOverlay::create();
		if (m_rOverlays)
		{
			m_rOverlays->retain();
			m_rOverlays->registerWithTouchDispatcher();
			addChild(m_rOverlays);
		}
	}

	return m_rOverlays;
}


void CCRichNode::processRichString(const char* utf8_str)
{
	if ( !utf8_str )
		return;

	element_list_t* eles = getParser()->parseString(utf8_str);
	if ( !eles )
		return;

	for ( element_list_t::iterator it = eles->begin(); it != eles->end(); it++ )
	{
		getCompositor()->composit(*it);
	}

	m_rElements.insert(m_rElements.end(), eles->begin(), eles->end());
	CC_SAFE_DELETE(eles);

	updateContentSize();
}

void CCRichNode::updateAll()
{
	clearStates();
	if ( !m_rRichString.empty() )
		processRichString(m_rRichString.c_str());
}

void CCRichNode::updateContentSize()
{
	if ( m_rContainer )
	{
		RRect rect = getCompositor()->getRect();
		m_rContainer->setContentSize(CCSize(/*m_rPreferedSize.w*/rect.size.w, rect.size.h));
		setPositionY(rect.size.h);
	}
}

void CCRichNode::clearStates()
{
	getCompositor()->reset();
	clearRichElements();

	// clear atlas
	clearAtlasMap();

	// clear overlays
	if (m_rOverlays)
	{
		m_rOverlays->reset();
		m_rOverlays->removeAllChildren();
	}

	updateContentSize();
}

void CCRichNode::clearRichElements()
{
	for ( element_list_t::iterator it = m_rElements.begin(); it != m_rElements.end(); it++ )
	{
		delete *it;
	}
	m_rElements.clear();
}

void CCRichNode::clearAtlasMap()
{
	for ( color_map_t::iterator color_it = m_rAtlasMap.begin(); color_it != m_rAtlasMap.end(); color_it++ )
	{
		for ( atlas_map_t::iterator atlas_it = color_it->second->begin(); atlas_it != color_it->second->end(); atlas_it++ )
		{
			CC_SAFE_RELEASE(atlas_it->second);
		}
		color_it->second->clear();
		delete color_it->second;
	}
	m_rAtlasMap.clear();

	for ( std::vector<CCRichAtlas*>::iterator it = m_rAtlasList.begin(); it != m_rAtlasList.end(); it++ )
	{
		CCRichAtlas* atlas = *it;
		CC_SAFE_RELEASE(atlas);
		getOverlay()->removeChild(atlas);
	}
	m_rAtlasList.clear();
}

CCRichAtlas* CCRichNode::findColoredTextureAtlas(CCTexture2D* texture, unsigned int color_rgba, int zorder)
{
	if ( texture == NULL || color_rgba == 0 )
	{
		return false;
	}

	atlas_map_t* atlas_map = NULL;
	color_map_t::iterator cit = m_rAtlasMap.find(color_rgba);
	if ( cit == m_rAtlasMap.end() )
	{
		atlas_map = new atlas_map_t;
		m_rAtlasMap.insert(std::make_pair(color_rgba, atlas_map));
	}
	else
	{
		atlas_map = cit->second;
	}

	CCRichAtlas* atlas = NULL;
	atlas_map_t::iterator ait = atlas_map->find(texture);
	if ( ait == atlas_map->end() )
	{
		atlas = CCRichAtlas::create(this, texture, m_rElements.size());
		cocos2d::ccColor3B color = ccc3( color_rgba & 0xff, color_rgba >> 8 & 0xff, color_rgba >> 16 & 0xff);
		atlas->setColor(color);
		atlas->setOpacity(color_rgba >> 24 & 0xff);

		atlas->retain();
		atlas_map->insert(std::make_pair(texture, atlas));
		atlas->retain();
		m_rAtlasList.push_back(atlas);

		getOverlay()->addChild(atlas, zorder);
	}
	else
	{
		atlas = ait->second;
	}

	return atlas;
}

void CCRichNode::draw()
{
	RRichCanvas canvas;
	canvas.root = this;
	canvas.rect/*.size*/ = getCompositor()->getRect()/*.size*/;

	for ( element_list_t::iterator it = m_rElements.begin(); it != m_rElements.end(); it++ )
	{
		(*it)->render(canvas);
	}
}

void CCRichNode::setPlainMode(bool on) 
{ 
	if ( getParser()->isPlainMode() != on )
	{
		getParser()->setPlainMode(on); 

		updateAll();
	}
}


CCRichNode::CCRichNode(class CCNode* container)
: m_rContainer(container)
, m_rParser(NULL)
, m_rCompositor(NULL)
, m_rOverlays(NULL)
{
}

CCRichNode::~CCRichNode()
{
	clearAtlasMap();
	clearRichElements();

	if ( m_rOverlays )
	{
		CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(m_rOverlays);
	}

	CC_SAFE_RELEASE(m_rOverlays);
	CC_SAFE_DELETE(m_rParser);
	CC_SAFE_DELETE(m_rCompositor);
}

CCHTMLNode* CCHTMLNode::createWithContainer(class CCNode* container)
{
	CCHTMLNode* node = new CCHTMLNode(container);
	if ( node->initialize() )
	{
		node->autorelease();
		return node;
	}

	CC_SAFE_DELETE(node);
	return NULL;
}

bool CCHTMLNode::initialize()
{
	m_rParser = new RSimpleHTMLParser(this);
	m_rCompositor = new RSimpleHTMLCompositor(this);

	return m_rParser && m_rCompositor;
}

CCHTMLNode::CCHTMLNode(class CCNode* container)
	: CCRichNode(container)
{

}

NS_CC_EXT_END;
