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
#include "CCRichAtlas.h"

NS_CC_EXT_BEGIN;

CCRichAtlas* CCRichAtlas::create(class IRichNode* container, CCTexture2D* texture, size_t capacity)
{
	CCRichAtlas* atlas = new CCRichAtlas(container);
	if ( atlas && atlas->initWithTexture(texture, capacity) )
	{
		atlas->autorelease();
		return atlas;
	}

	CC_SAFE_DELETE(atlas);
	return NULL;
}

bool CCRichAtlas::initWithTexture(CCTexture2D* texture, size_t capacity)
{
	CCAtlasNode::initWithTexture(texture, 0, 0, capacity);

	return true;
}

void CCRichAtlas::appendRichElement(IRichElement* element)
{
	m_elements.push_back(element);
	setQuadsToDraw(getQuadsToDraw()+1);
	m_dirty = true;
}

void CCRichAtlas::resetRichElements()
{
	reset();
}

void CCRichAtlas::updateRichRenderData()
{
	updateAtlasValues();
}


void CCRichAtlas::resizeCapacity(size_t ns)
{
	m_pTextureAtlas->resizeCapacity(ns);
}

void CCRichAtlas::reset()
{
	m_elements.clear();
	setQuadsToDraw(0);
	m_dirty = true;

	m_pTextureAtlas->removeAllQuads();
}

void CCRichAtlas::updateAtlasValues()
{
	if ( !m_dirty )
	{
		return;
	}
	m_dirty = false;

	if ( m_pTextureAtlas->getCapacity() < getQuadsToDraw() )
	{
		m_pTextureAtlas->resizeCapacity( getQuadsToDraw() );
	}

	ccV3F_C4B_T2F_Quad quad;

	CCTexture2D *texture = m_pTextureAtlas->getTexture();
	float textureWide = (float) texture->getPixelsWide();
	float textureHigh = (float) texture->getPixelsHigh();

	int i = 0;
	for(std::list<IRichElement*>::iterator it = m_elements.begin(); 
		it != m_elements.end(); it++, i++) {
			IRichElement* ele = *it;
			RTexture* rtex = ele->getTexture();

#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
			float left   = (rtex->rect.pos.x + 0.5f)/textureWide;
			float right  = left + (rtex->rect.size.w - 1.0f)/textureWide;
			float top    = (rtex->rect.pos.y + 0.5f)/textureHigh;
			float bottom = top + (rtex->rect.size.h - 1.0f)/textureHigh;

			float ele_pos_left = ele->getGlobalPosition().x;
			float ele_pos_top = ele->getGlobalPosition().y;
			float ele_width = ele->scaleToElementSize() ? 
				ele->getMetrics()->rect.size.w : rtex->rect.size.w;
			float ele_height = ele->scaleToElementSize() ? 
				ele->getMetrics()->rect.size.h : rtex->rect.size.h;
#else
			float left   = rtex->rect.pos.x/textureWide;
			float right  = left + rtex->rect.size.w/textureWide;
			float top    = rtex->rect.pos.y/textureHigh;
			float bottom = top + rtex->rect.size.h/textureHigh;

			float ele_pos_left = ele->getGlobalPosition().x;
			float ele_pos_top = ele->getGlobalPosition().y;
			float ele_width = ele->scaleToElementSize() ? 
				ele->getMetrics()->rect.size.w : rtex->rect.size.w;
			float ele_height = ele->scaleToElementSize() ? 
				ele->getMetrics()->rect.size.h : rtex->rect.size.h;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

			quad.tl.texCoords.u = left;
			quad.tl.texCoords.v = top;
			quad.tr.texCoords.u = right;
			quad.tr.texCoords.v = top;
			quad.bl.texCoords.u = left;
			quad.bl.texCoords.v = bottom;
			quad.br.texCoords.u = right;
			quad.br.texCoords.v = bottom;

			quad.bl.vertices.x = (float) (ele_pos_left);
			quad.bl.vertices.y = ele_pos_top - ele_height;
			quad.bl.vertices.z = 0.0f;
			quad.br.vertices.x = (float)(ele_pos_left + ele_width);
			quad.br.vertices.y = ele_pos_top - ele_height;
			quad.br.vertices.z = 0.0f;
			quad.tl.vertices.x = (float)(ele_pos_left);
			quad.tl.vertices.y = ele_pos_top;
			quad.tl.vertices.z = 0.0f;
			quad.tr.vertices.x = (float)(ele_pos_left + ele_width);
			quad.tr.vertices.y = ele_pos_top;
			quad.tr.vertices.z = 0.0f;

			ccColor4B c = { _displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity };
			quad.tl.colors = c;
			quad.tr.colors = c;
			quad.bl.colors = c;
			quad.br.colors = c;
			m_pTextureAtlas->updateQuad(&quad, i);
	}
}

void CCRichAtlas::draw()
{
	if ( m_dirty )
	{
		this->updateRichRenderData();
	}

	CCAtlasNode::draw();

#if CCRICH_DEBUG
	// atlas bounding box
	const CCSize& s = this->getContentSize();
	CCPoint vertices[4]={
		ccp(0,0),ccp(s.width,0),
		ccp(s.width,s.height),ccp(0,s.height),
	};
	ccDrawColor4B(0x00, 0xff, 0x00, 0xff);
	ccDrawPoly(vertices, 4, true);
#endif
}

CCRichAtlas::CCRichAtlas(class IRichNode* container)
: m_container(container)
, m_dirty(true)
{
}

CCRichAtlas::~CCRichAtlas()
{ 
	m_elements.clear();
}

NS_CC_EXT_END;
