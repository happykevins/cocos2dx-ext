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
#include "CCRichElement.h"

#include <cocos-ext.h>

using namespace dfont;

NS_CC_EXT_BEGIN;

//////////////////////////////////////////////////////////////////////////
// REleBase

REleBase::attrs_t* REleBase::parseAttributes(const char** attrs)
{
	attrs_t* attrmap = new attrs_t;

	if ( attrs )
	{
		for ( size_t i = 0; attrs[i] != NULL && attrs[i+1] != NULL; i+=2 )
		{
			attrmap->insert(std::make_pair<std::string, std::string>(attrs[i], attrs[i+1]));
		}
	}

	return attrmap;
}

bool REleBase::hasAttribute(attrs_t* attrs, const char* attr)
{
	CCAssert(attr, "");

	attrs_t::iterator it = attrs->find(attr);
	if ( it != attrs->end() )
	{
		return true;
	}

	return false;
}

CCNode* REleBase::createDrawSolidPolygonNode(RRichCanvas canvas)
{
	CCDrawNode* drawNode = CCDrawNode::create();

	RRect rect = m_rMetrics.rect;
	RPos gp = getGlobalPosition();
	short left = gp.x;
	short top = gp.y;// + canvas.root->getActualSize().h;
	short right = left + rect.size.w;
	short bottom = top - rect.size.h;

	CCPoint vertices[4] = {
		ccp(left, bottom),	// lb
		ccp(right,bottom),	// rb
		ccp(right,top),		// rt
		ccp(left,top),		// lt
	};

	unsigned int color = getColor();
	ccColor4F color4f = ccc4FFromccc4B(ccc4(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff, color >> 24 & 0xff));

	drawNode->drawPolygon(vertices, 4, color4f, 0.0f, color4f);

	return drawNode;
}

bool REleBase::parse(class IRichParser* parser, const char** attr /*= NULL*/) 
{ 
	attrs_t* attrs = parseAttributes(attr);
	CCAssert(attrs, "");

	if ( hasAttribute(attrs, "id") )
	{
		m_rID = atoi( (*attrs)["id"].c_str() );
	}

	bool success = onParseAttributes(parser, attrs);

	CC_SAFE_DELETE(attrs);

	return success; 
}

bool REleBase::composit(class IRichCompositor* compositor) 
{
	// set position 
	RMetricsState* state = compositor->getMetricsState();

	m_rPos.x = state->pen_x;
	m_rPos.y = state->pen_y;

	onCompositStart(compositor);

	if ( pushMetricsState() )
	{
		compositor->pushMetricsState();

		// calculate new zone position
		RMetricsState* mstate = compositor->getMetricsState();
		mstate->zone.pos.add(m_rPos);
		mstate->pen_x = 0;
		mstate->pen_y = 0;
	}

	if ( pushRenderState() )
	{
		compositor->pushRenderState();
	}

	onCompositStatePushed(compositor);

	RRect minrect;
	element_list_t* children = getChildren();
	if ( children )
	{
		for ( element_list_t::iterator it = children->begin(); it != children->end(); it++ )
		{
			(*it)->composit(compositor);
			if ( !(*it)->isCachedComposit() )
			{
				RRect subrect = (*it)->getMetrics()->rect;
				// add parent pos only if pushed metrics
				if ( pushMetricsState() )
					subrect.pos.add((*it)->getLocalPosition());
				minrect.extend(subrect);
			}
		}
	}

	// flush last line
	if ( getParent() == NULL )
	{
		RRect subrect = compositor->getMetricsState()->elements_cache->flush(compositor);
		minrect.extend(subrect);
	}

	onCompositChildrenEnd(compositor);

	if ( pushRenderState() )
	{
		compositor->popRenderState();
	}
	if ( pushMetricsState() )
	{
		compositor->popMetricsState();
	}

	// set rect
	m_rMetrics.rect.extend(minrect);


	if ( onCompositFinish(compositor) )
	{
		if ( isCachedComposit() )
		{
			compositor->getMetricsState()->elements_cache->appendElement(this);
		}
		else
		{
			// advance pen
			state->pen_x += m_rMetrics.advance.x;
			state->pen_y += m_rMetrics.advance.y;
		}
	}

	return true;
}

void REleBase::render(RRichCanvas canvas)
{
	// calculate global position
	m_rGlobalPos.x = m_rPos.x + canvas.rect.pos.x + m_rMetrics.rect.pos.x;
	m_rGlobalPos.y = m_rPos.y + canvas.rect.pos.y + m_rMetrics.rect.pos.y;

	onRenderPrev(canvas);

	element_list_t* children = getChildren();
	if ( children )
	{
		RRichCanvas push_canvas = canvas;

		if ( pushMetricsState() )
		{
			push_canvas.rect.pos = m_rGlobalPos;
			push_canvas.rect.size = m_rMetrics.rect.size;
		}

		for ( element_list_t::iterator it = children->begin(); it != children->end(); it++ )
		{
			(*it)->render(push_canvas);
		}
	}

	onRenderPost(canvas);

#if CCRICH_DEBUG
	// bounding box
	RRect rect = m_rMetrics.rect;
	RPos gp = getGlobalPosition();
	short left = gp.x;
	short top = gp.y;// + canvas.root->getActualSize().h;
	short right = left + rect.size.w;
	short bottom = top - rect.size.h;
	CCPoint vertices[4]={
		ccp(left,bottom),ccp(right,bottom),
		ccp(right,top),ccp(left,top),
	};
	ccDrawColor4B(0xff, 0xff, 0x00, 0xff);
	ccDrawPoly(vertices, 4, true);
#endif
}

// children
element_list_t* REleBase::getChildren()
{
	return m_rChildren;
}

void REleBase::addChildren(IRichElement* child)
{
	CCAssert ( child && child->getParent() == NULL, "[CCRich]invalid element!" );

	// lazy init
	if ( m_rChildren == NULL )
	{
		m_rChildren = new element_list_t;
	}

	m_rChildren->push_back(child);
	child->setParent(this);
}

void REleBase::removeAllChildren()
{
	if ( m_rChildren )
	{
		for ( element_list_t::iterator it = m_rChildren->begin(); it != m_rChildren->end(); it++ )
		{
			delete *it;
		}
		m_rChildren->clear();
		CC_SAFE_DELETE(m_rChildren);
	}
}

IRichElement* REleBase::getParent()
{
	return m_rParent;
}

void REleBase::setParent(IRichElement* parent)
{
	m_rParent = parent;
}

int	REleBase::getID()
{
	return m_rID;
}

IRichElement* REleBase::findChildByID(int _id)
{
	if ( m_rID == _id )
		return this;

	element_list_t* children = getChildren();
	if ( getChildren() )
	{
		for ( element_list_t::iterator it = children->begin(); it != children->end(); it++ )
		{
			IRichElement* child = (*it)->findChildByID(_id);
			if ( child )
				return child;
		}
	}

	return NULL;
}

REleBase::REleBase()
: m_rID(0)
, m_rChildren(NULL)
, m_rParent(NULL)
, m_rPos()
, m_rGlobalPos()
, m_rMetrics()
, m_rTexture()
, m_rColor(0xffffffff)
, m_rDirty(false)
{

}

REleBase::~REleBase()
{
	removeAllChildren();
	m_rParent = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Common Elements

CCNode* REleSolidPolygon::createDrawNode(RRichCanvas canvas)
{
	this->render(canvas);
	return createDrawSolidPolygonNode(canvas);
}
/**
void REleSolidPolygon::onRenderPost(RRichCanvas canvas)
{
	RRect rect = m_rMetrics.rect;
	RPos gp = getGlobalPosition();
	short left = gp.x;
	short top = gp.y;// + canvas.root->getActualSize().h;
	short right = left + rect.size.w;
	short bottom = top - rect.size.h;

	CCPoint vertices[4] = {
		ccp(left, bottom),	// lb
		ccp(right,bottom),	// rb
		ccp(right,top),		// rt
		ccp(left,top),		// lt
	};

	unsigned int color = getColor();
	ccColor4F color4f = ccc4FFromccc4B(ccc4(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff, color >> 24 & 0xff));
	ccDrawSolidPoly(vertices, 4, color4f);
}
*/
bool REleBatchedDrawable::onCompositFinish(class IRichCompositor* compositor) 
{
	m_rDirty = true;

	return true;
}

void REleBatchedDrawable::onRenderPrev(RRichCanvas canvas)
{
	if ( m_rDirty )
	{
		m_rDirty = false;

		// add to batch
		CCTexture2D* ob_texture = NULL;
		if ( NULL != this->getTexture() 
			&& NULL != (ob_texture = this->getTexture()->getTexture()) )
		{
			IRichAtlas* atlas = canvas.root->findAtlas(ob_texture, getColor());

			if (atlas)
			{
				atlas->appendRichElement(this);
			}
		}
	}
}

REleBatchedDrawable::REleBatchedDrawable()
{

}

void RAtlasHelper::onRenderPrev(RRichCanvas canvas)
{
	if ( m_rDirty )
	{
		m_rDirty = false;

		// add to batch
		CCTexture2D* ob_texture = NULL;
		if ( NULL != this->getTexture() 
			&& NULL != (ob_texture = this->getTexture()->getTexture()) )
		{
			IRichAtlas* atlas = canvas.root->findAtlas(ob_texture, getColor(), ZORDER_BACKGROUND);

			if (atlas)
			{
				atlas->appendRichElement(this);
			}
		}
	}
}

void REleGlyph::onCompositStart(class IRichCompositor* compositor)
{
	if ( !compositor->getFont() )
		return;

	m_slot = compositor->getFont()->require_char(m_charcode);

	if ( m_slot )
	{
		m_rMetrics.rect.pos = RPos((short)m_slot->metrics.left, (short)m_slot->metrics.top);
		m_rMetrics.rect.size = RSize((short)m_slot->metrics.width, (short)m_slot->metrics.height);
		m_rMetrics.advance.x = m_slot->metrics.advance_x;
		m_rMetrics.advance.y = 0;//m_slot->metrics.advance_y;

		m_rTexture.setTexture(m_slot->texture->user_texture<CCTexture2D>());
		m_rTexture.rect.pos = RPos((short)m_slot->padding_rect.origin_x, (short)m_slot->padding_rect.origin_y);
		m_rTexture.rect.size = RSize((short)m_slot->padding_rect.width, (short)m_slot->padding_rect.height);

		RRenderState* state = compositor->getRenderState();
		m_font_alias = state->font_alias;
		m_rColor = state->color;
	}
}

REleGlyph::REleGlyph(unsigned int charcode)
	: m_charcode(charcode), m_slot(NULL)
{

}

REleGlyph::~REleGlyph()
{
	CC_SAFE_RELEASE( m_slot );
}

//////////////////////////////////////////////////////////////////////////
// REleHTML

REleBase::attrs_t* REleHTMLNode::parseStyle(const std::string& style_str)
{
	attrs_t* attrs = new attrs_t;
	if ( style_str.empty() )
		return attrs;

	size_t start_pos = 0;
	size_t end_pos = 0;
	size_t sep_pos = std::string::npos;
	while (  start_pos < style_str.size() )
	{
		end_pos = style_str.size();
		sep_pos = style_str.find_first_of(';', start_pos);
		if ( sep_pos != std::string::npos )
			end_pos = sep_pos;

		std::string phase = style_str.substr(start_pos, end_pos - start_pos);
		start_pos = style_str.find_first_not_of("; ", end_pos);

		size_t kv_pos = phase.find_first_of(":");
		if ( kv_pos != std::string::npos )
		{
			size_t v_start = phase.find_first_not_of(": ", kv_pos);
			size_t v_last = phase.find_last_not_of(' ');
			(*attrs)[phase.substr(0, kv_pos)] = phase.substr(v_start, v_last - v_start + 1);
		}
	}

	return attrs;
}

RMargin REleHTMLNode::parseMargin(const std::string& str)
{
	RMargin margin;

	size_t start_pos = 0;
	size_t sep_pos = 0;

	start_pos = str.find_first_not_of(' ', sep_pos);
	sep_pos = str.find_first_of(' ', start_pos);
	std::string phase = str.substr(start_pos, sep_pos - start_pos);
	margin.top = parsePixel(phase);

	if ( sep_pos == std::string::npos )
		return margin;
	
	start_pos = str.find_first_not_of(' ', sep_pos);
	sep_pos = str.find_first_of(' ', start_pos);
	phase = str.substr(start_pos, sep_pos - start_pos);
	margin.right = parsePixel(phase);
	
	if ( sep_pos == std::string::npos )
		return margin;

	start_pos = str.find_first_not_of(' ', sep_pos);
	sep_pos = str.find_first_of(' ', start_pos);
	phase = str.substr(start_pos, sep_pos - start_pos);
	margin.bottom = parsePixel(phase);

	if ( sep_pos == std::string::npos )
		return margin;

	start_pos = str.find_first_not_of(' ', sep_pos);
	sep_pos = str.find_first_of(' ', start_pos);
	phase = str.substr(start_pos, sep_pos - start_pos);
	margin.left = parsePixel(phase);

	return margin;
}

unsigned int REleHTMLNode::parseColor(const std::string& color_str)
{
	unsigned int color = 0;
	if ( !color_str.empty() )
	{
		if ( color_str[0] == '#' 
			&& (color_str.size() == 7 || color_str.size() == 9) ) // RGB || RGBA
		{
			if ( color_str.size() == 7 )
			{
				color = 0xff;
			}

			for ( size_t i = color_str.size() - 1; i > 1; i-=2 )
			{
				color = (color<<4) + cc_transfer_hex_value(color_str[i-1]);
				color = (color<<4) + cc_transfer_hex_value(color_str[i]);
			}
		}
	}

	return color;
}

ROptSize REleHTMLNode::parseOptSize(const std::string& str)
{
	ROptSize size;
	size.ratio = parsePercent(str);
	if ( 0 == size.ratio )
	{
		size.absolute = parsePixel(str);
	}
	return size;
}

short REleHTMLNode::parsePixel(const std::string& str)
{
	size_t last_pos = str.find_last_not_of(" px"); // skip px
	short pixels = atoi(str.substr(0, last_pos + 1).c_str());
	return pixels;
}

float REleHTMLNode::parsePercent(const std::string& str)
{
	float ratio = 0.0f;
	if ( !str.empty() && str[str.size()-1] == '%' )
	{
		std::string value_part = str.substr(0, str.size() - 1);
		ratio = (float)atof(value_part.c_str());
		ratio *= 0.01f;
	}
	return ratio;
}

bool REleHTMLNode::parseAlignment(const std::string& str, EAlignment& align)
{
	if ( str.empty() )
		return false;

	if ( strcmp(str.c_str(), "left") == 0 )
	{
		align = e_align_left;
	}
	else if ( strcmp(str.c_str(), "right") == 0 )
	{
		align = e_align_right;
	}
	else if ( strcmp(str.c_str(), "center") == 0 )
	{
		align = e_align_center;
	}
	else if ( strcmp(str.c_str(), "top") == 0 )
	{
		align = e_align_top;
	}
	else if ( strcmp(str.c_str(), "bottom") == 0 )
	{
		align = e_align_bottom;
	}
	else if ( strcmp(str.c_str(), "middle") == 0 )
	{
		align = e_align_middle;
	}
	else
	{
		return false;
	}

	return true;
}

//void REleHTMLNode::processZone(RRect& zone, const ROptSize& width, const ROptSize& height, bool auto_fill_zone/*=false*/)
//{
//	short target_width = 0;
//	short target_height = 0;
//	if ( width.ratio )
//	{
//		target_width = zone.size.w * width.ratio;
//	}
//	if ( height.ratio )
//	{
//		target_height = zone.size.h * height.ratio;
//	}
//
//	target_width = RMAX(target_width, width.absolute);
//	target_height = RMAX(target_height, height.absolute);
//
//	// size == 0 represent auto grow
//	if ( zone.size.w == 0 )
//	{
//		zone.size.w = target_width;
//	}
//	else if ( auto_fill_zone )
//	{
//		zone.size.w = target_width == 0 ? 0 : RMAX(zone.size.w, target_width);
//	}
//	else// not auto_fill_zone
//	{
//		zone.size.w = target_width > 0 ? target_width : zone.size.w;
//	}
//
//	if ( zone.size.h == 0 )
//	{
//		zone.size.h = target_height;
//	}
//	else if ( auto_fill_zone )
//	{
//		zone.size.h = target_height == 0 ? 0 : RMAX(zone.size.h, target_height);
//	}
//	else// not auto_fill_zone
//	{
//		zone.size.h = target_height > 0 ? target_height : zone.size.h;
//	}
//}

bool REleHTMLP::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	REleBase::attrs_t* style_attrs = NULL;

	if ( hasAttribute(attrs, "style") )
	{
		style_attrs = parseStyle((*attrs)["style"]);

		// parse alignment
		if ( hasAttribute(style_attrs, "text-align") )
		{
			EAlignment align = e_align_left;
			parseAlignment((*style_attrs)["text-align"], align);
			m_rLineCache.setHAlign(align);
		}
		
		// parse wrap line
		if ( hasAttribute(style_attrs, "white-space") )
		{
			if ( strcmp( (*style_attrs)["white-space"].c_str(), "nowrap") == 0 )
			{
				m_rLineCache.setWrapline(false);
			}
			else
			{
				m_rLineCache.setWrapline(true);
			}
		}

		// color
		m_rColor = parseColor((*style_attrs)["color"]);

		// font
		m_rFontAlias = (*style_attrs)["font"];

		// line-height
		if ( hasAttribute(style_attrs, "line-height") )
			m_rLineCache.setLineHeight( parsePixel((*style_attrs)["line-height"]) );

		// margin
		if ( hasAttribute(style_attrs, "margin") )
		{
			RMargin margin = parseMargin((*style_attrs)["margin"]);
			m_rLineCache.setSpacing(margin.top);
		}

		// padding
		if ( hasAttribute(style_attrs, "padding") )
		{
			RMargin padding = parseMargin((*style_attrs)["padding"]);
			m_rLineCache.setPadding(padding.top);
		}

		CC_SAFE_DELETE(style_attrs);
	}

	return true;
}

void REleHTMLP::onCompositStatePushed(class IRichCompositor* compositor)
{
	RMetricsState* mstate = compositor->getMetricsState();
	mstate->elements_cache = &m_rLineCache;

	if ( m_rColor )
	{
		compositor->getRenderState()->color = m_rColor;
	}

	if ( !m_rFontAlias.empty() )
	{
		compositor->getRenderState()->font_alias = m_rFontAlias.c_str();
	}
}

void REleHTMLP::onCompositChildrenEnd(class IRichCompositor* compositor)
{
	RRect rect = m_rLineCache.flush(compositor);
	m_rMetrics.rect.extend(rect);
}

bool REleHTMLP::onCompositFinish(class IRichCompositor* compositor) 
{
	return true; 
}


bool REleHTMLBR::onCompositFinish(class IRichCompositor* compositor)
{
	m_rMetrics.rect.size.h = compositor->getFont()->char_height();
	m_rMetrics.rect.pos.y = m_rMetrics.rect.size.h;
	return true;
}

bool REleHTMLFont::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	m_rFont = (*attrs)["face"];
	m_rColor = parseColor((*attrs)["color"]);

	return true;
}

REleHTMLFont::REleHTMLFont()
	: m_rColor(0)
{

}

void REleHTMLFont::onCompositStatePushed(class IRichCompositor* compositor)
{
	if ( !m_rFont.empty() )
		compositor->getRenderState()->font_alias = m_rFont.c_str();

	if ( m_rColor != 0 )
		compositor->getRenderState()->color = m_rColor;
}


void REleHTMLSpans::travesalChildrenSpans(
	element_list_t* eles, const char*& font, short& start_span_x, short& span_y, short& underline_thickness, 
	short& end_span_x, short& min_span_y, short& max_span_y, unsigned int& color, bool is_root)
{
	if ( !eles )
		return;

	for ( element_list_t::iterator it = eles->begin(); it != eles->end(); it++  )
	{
		if ( !(*it)->pushMetricsState() )
		{
			travesalChildrenSpans(
				(*it)->getChildren(), font,
				start_span_x, span_y, underline_thickness,
				end_span_x, min_span_y, max_span_y, color, false);
		}

		bool process_begin_after_end = false;
		bool process_end = false;
		bool process_advance = false;
		
		RMetrics* metrics = (*it)->getMetrics();
		RPos pos = (*it)->getLocalPosition();

		short this_thickness = metrics->rect.size.h / 20 + 1;
		unsigned int this_color = (*it)->getColor();
		const char* this_font = (*it)->getFontAlias();

		if ( this_font )
		{
			// no underline before
			if ( font == NULL ) 
			{
				// #issue: only one character cause none span created
				start_span_x = pos.x + metrics->rect.pos.x;
				span_y = pos.y;
				underline_thickness = this_thickness;
				end_span_x = start_span_x + metrics->rect.size.w;
				min_span_y = metrics->rect.min_y();
				max_span_y = metrics->rect.max_y();
				color = (*it)->getColor();
				font = this_font;
			}
			// advance underline
			else if ( span_y == pos.y && strcmp(this_font, font) == 0 && this_color == color )
			{
				process_advance = true;
			}
			// meet a new underline
			else
			{
				process_end = true;
				process_begin_after_end = true;
			}
		}
		else
		{
			// finish a underline
			if ( font )
			{
				process_end = true;
			}

			// add a single span for none-character element
			if ( metrics->rect.size.w != 0 && metrics->rect.size.h != 0 )
			{
				RRect span;
				span.pos.x = pos.x + metrics->rect.pos.x;
				span.pos.y = pos.y + metrics->rect.pos.y;
				span.size = RSize(metrics->rect.size.w, metrics->rect.size.h);
				m_rSpans.push_back(span);

				REleSolidPolygon* background_drawable = new REleSolidPolygon;
				background_drawable->setLocalPosition(span.pos);
				background_drawable->getMetrics()->rect.size = span.size;
				background_drawable->setRColor(getBGColor());
				m_rBackgroudDrawables.push_back(background_drawable);
			}
		}

		// the last element
		if ( it+1 == eles->end() && is_root)
		{
			process_end = true;
		}

		// process advance
		if ( process_advance )
		{
			min_span_y = RMIN(min_span_y, metrics->rect.min_y());
			max_span_y = RMAX(max_span_y, metrics->rect.max_y());
			end_span_x = pos.x + metrics->rect.pos.x + metrics->rect.size.w;
			underline_thickness = RMAX(underline_thickness, this_thickness);
		}

		// process end
		if ( process_end && font )
		{
			REleSolidPolygon* underline_drawable = new REleSolidPolygon;
			underline_drawable->setLocalPosition(RPos(start_span_x, span_y + min_span_y));
			underline_drawable->getMetrics()->rect.size = RSize(end_span_x - start_span_x, underline_thickness);
			underline_drawable->setRColor(color);
			m_rUnderlineDrawables.push_back(underline_drawable);

			RRect span;
			span.pos.x = start_span_x;
			span.pos.y = span_y + max_span_y;
			span.size = RSize(end_span_x - start_span_x, max_span_y - min_span_y);
			m_rSpans.push_back(span);

			REleSolidPolygon* background_drawable = new REleSolidPolygon;
			background_drawable->setLocalPosition(span.pos);
			background_drawable->getMetrics()->rect.size = span.size;
			background_drawable->setRColor(getBGColor());
			m_rBackgroudDrawables.push_back(background_drawable);

			start_span_x = 0;
			end_span_x = 0;
			underline_thickness = 0;
			max_span_y = 0;
			font = NULL;
		}

		// process begin
		if ( process_begin_after_end )
		{
			start_span_x = pos.x + metrics->rect.pos.x;
			span_y = pos.y;
			underline_thickness = this_thickness;
			end_span_x = start_span_x + metrics->rect.size.w;
			min_span_y = metrics->rect.min_y();
			max_span_y = metrics->rect.max_y();
			color = (*it)->getColor();
			font = this_font;
		}
	}
}

void REleHTMLSpans::onRenderPost(RRichCanvas canvas) 
{
	// create underline drawable
	if ( m_rDirty )
	{
		clearAllSpans();

		short start_pen_x = 0;
		short pen_y = 0;
		short thickness = 0;
		short end_draw_x = 0;
		short draw_y = 0;
		short max_height = 0;
		unsigned int color = 0;
		const char* font = 0;

		travesalChildrenSpans(
			this->getChildren(), font, start_pen_x, pen_y, thickness, end_draw_x, draw_y, max_height, color, true);

		// draw underline
		if ( isDrawUnderline() )
		{
			for ( size_t i = 0; i < m_rUnderlineDrawables.size(); i++ )
			{
				CCNode* drawNode = m_rUnderlineDrawables[i]->createDrawNode(canvas);
				drawNode->setZOrder(ZORDER_OVERLAY);
				canvas.root->addCCNode(drawNode);
			}
		}

		// draw background
		if ( isDrawBackground() )
		{
			for ( size_t i = 0; i < m_rBackgroudDrawables.size(); i++ )
			{
				CCNode* drawNode = m_rBackgroudDrawables[i]->createDrawNode(canvas);
				drawNode->setZOrder(ZORDER_OVERLAY);
				canvas.root->addCCNode(drawNode);
			}
		}

		m_rDirty = false;
	}
	
	//// draw underline
	//if ( isDrawUnderline() )
	//{
	//	for ( size_t i = 0; i < m_rUnderlineDrawables.size(); i++ )
	//	{
	//		m_rUnderlineDrawables[i]->render(canvas);
	//	}
	//}

	//// draw background
	//if ( isDrawBackground() )
	//{
	//	for ( size_t i = 0; i < m_rBackgroudDrawables.size(); i++ )
	//	{
	//		m_rBackgroudDrawables[i]->render(canvas);
	//	}
	//}
}

void REleHTMLSpans::clearAllSpans()
{
	for ( size_t i = 0; i < m_rUnderlineDrawables.size(); i++ )
	{
		CC_SAFE_DELETE(m_rUnderlineDrawables[i]);
	}
	m_rUnderlineDrawables.clear();

	for ( size_t i = 0; i < m_rBackgroudDrawables.size(); i++ )
	{
		CC_SAFE_DELETE(m_rBackgroudDrawables[i]);
	}
	m_rBackgroudDrawables.clear();

	m_rSpans.clear();
}

REleHTMLSpans::REleHTMLSpans()
	: m_rDrawUnderline(false), m_rDrawBackground(false), m_rBGColor(0xffffffff)
{
	m_rDirty = true;
}

REleHTMLSpans::~REleHTMLSpans()
{
	clearAllSpans();
}


REleHTMLU::REleHTMLU()
{
	setDrawUnderline(true);
}


bool REleHTMLHR::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	unsigned int color = 0;

	m_rSize = REleHTMLNode::parsePixel((*attrs)["size"]);
	m_rWidth = REleHTMLNode::parseOptSize( (*attrs)["width"] );

	if ( m_rSize == 0 )
	{
		m_rSize = 1;
	}

	if ( m_rWidth.absolute == 0 && m_rWidth.ratio == 0 )
	{
		m_rWidth.ratio = 1.0f;
	}

	if ( hasAttribute(attrs, "style") )
	{
		attrs_t* style_attrs = REleHTMLNode::parseStyle((*attrs)["style"]);

		if ( hasAttribute(style_attrs, "color") )
		{
			unsigned int color = REleHTMLNode::parseColor( (*style_attrs)["color"] );
			m_rColor = color;
		}
		CC_SAFE_DELETE(style_attrs);
	}

	m_rDirty = true;

	return true;
}

void REleHTMLHR::onCachedCompositBegin(class ICompositCache* cache, RPos& pen)
{
	pen.y -= cache->getSpacing();
}

void REleHTMLHR::onCachedCompositEnd(class ICompositCache* cache, RPos& pen)
{
	pen.y -= cache->getSpacing();
}

void REleHTMLHR::onCompositStart(class IRichCompositor* compositor)
{
	RMetricsState* metrics = compositor->getMetricsState();
	m_rMetrics.rect.size.w = metrics->zone.size.w - metrics->elements_cache->getPadding() * 2;
	m_rMetrics.rect.size.w = m_rWidth.getValueReal(m_rMetrics.rect.size.w);
	m_rMetrics.rect.size.h = m_rSize;
	m_rMetrics.rect.pos.y = m_rMetrics.rect.size.h;

	if ( m_rColor == 0 )
		m_rColor = compositor->getRenderState()->color;
}

bool REleHTMLHR::onCompositFinish(class IRichCompositor* compositor)
{
	return true;
}

void REleHTMLHR::onRenderPost(RRichCanvas canvas)
{
	/**
	RRect rect = m_rMetrics.rect;
	RPos gp = getGlobalPosition();
	short left = gp.x;
	short top = gp.y;// + canvas.root->getActualSize().h;
	short right = left + rect.size.w;
	short bottom = top - rect.size.h;
	short bordersize = 1;

	CCPoint vertices[4] = {
		ccp(left, bottom),	// lb
		ccp(right,bottom),	// rb
		ccp(right,top),		// rt
		ccp(left,top),		// lt
	};

	unsigned int color = getColor();
	ccColor4F color4f = ccc4FFromccc4B(ccc4(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff, color >> 24 & 0xff));
	ccDrawSolidPoly(vertices, 4, color4f);

	CCPoint vertices_shadow[4] = {
		ccp(left, bottom - bordersize),	// lb
		ccp(right,bottom - bordersize),	// rb
		ccp(right,bottom),	// rt
		ccp(left,bottom),	// lt
	};
	color4f.r *= 0.2f;
	color4f.g *= 0.2f;
	color4f.b *= 0.2f;
	ccDrawSolidPoly(vertices_shadow, 4, color4f);	
	**/

	if (m_rDirty)
	{
		CCNode* drawNode = createDrawSolidPolygonNode(canvas);
		drawNode->setZOrder(ZORDER_CONTEXT);
		canvas.root->addCCNode(drawNode);
		m_rDirty = false;
	}
}

REleHTMLHR::REleHTMLHR()
	: m_rSize(1), m_rTempPadding(0)
{

}

//////////////////////////////////////////////////////////////////////////
// HTML Table

bool REleHTMLCell::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	m_rWidth = parseOptSize((*attrs)["width"]);
	m_rHeight = parseOptSize((*attrs)["height"]);

	m_rHAlignSpecified = parseAlignment((*attrs)["align"], m_rHAlignment);
	m_rVAlignSpecified = parseAlignment((*attrs)["valign"], m_rVAlignment);

	short padding = parsePixel((*attrs)["padding"]);
	short spacing = parsePixel((*attrs)["spacing"]);
	m_rLineCache.setPadding(padding);
	m_rLineCache.setSpacing(spacing);

	if ( strcmp((*attrs)["nowrap"].c_str(), "nowrap") == 0 )
	{
		m_rLineCache.setWrapline(false);
	}

	// color
	m_rColor = parseColor((*attrs)["bgcolor"]);

	m_rBGTexture.setDirty(false);
	if ( hasAttribute(attrs, "bg-image") )
	{
		std::string bg_filename = (*attrs)["bg-image"];
		CCTexture2D* texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(bg_filename.c_str());
		if ( texture )
		{
			m_rBGTexture.setDirty(true);
			if (m_rColor) 
			{
				m_rBGTexture.setRColor(m_rColor);
				m_rColor = 0;
			}
			RTexture* bg_texture = m_rBGTexture.getTexture();
			bg_texture->setTexture(texture);

			if ( hasAttribute(attrs, "bg-rect") )
			{
				RMargin margin = REleHTMLNode::parseMargin( (*attrs)["bg-rect"] );
				bg_texture->rect.pos.x = margin.left;
				bg_texture->rect.pos.y = margin.top;
				bg_texture->rect.size.h = margin.bottom - margin.top;
				bg_texture->rect.size.w = margin.right - margin.left;
			}
			else
			{
				bg_texture->rect.size.w = texture->getPixelsWide();
				bg_texture->rect.size.h = texture->getPixelsHigh();
			}
		}
	}

	m_rDirty = true;

	return true;
}

void REleHTMLCell::onRenderPrev(RRichCanvas canvas)
{
	// render background
	if ( m_rDirty || m_rBGTexture.isDirty() )
	{
		RRect rect = m_rMetrics.rect;
		RPos gp = getGlobalPosition();
		short left = gp.x;
		short top = gp.y;
		short right = left + rect.size.w;
		short bottom = top - rect.size.h - 1;

		if (m_rDirty && m_rColor)
		{
			CCNode* drawNode = createDrawSolidPolygonNode(canvas);
			drawNode->setZOrder(ZORDER_BACKGROUND);
			canvas.root->addCCNode(drawNode);
		}

		if ( m_rBGTexture.isDirty() )
		{
			m_rBGTexture.setGlobalPosition(gp);
			m_rBGTexture.getMetrics()->rect.size = RSize(rect.size.w, rect.size.h);
			m_rBGTexture.onRenderPrev(canvas);
		}

		m_rDirty = false;
	}
}

void REleHTMLCell::onCompositStatePushed(class IRichCompositor* compositor)
{
	RMetricsState* mstate = compositor->getMetricsState();
	mstate->elements_cache = &m_rLineCache;

	mstate->zone.size.w = m_rRow->getCellWidth(m_rIndexNumber, m_rWidth);
	mstate->zone.size.h = m_rHeight.getValueReal(mstate->zone.size.h);
	m_rMetrics.rect.size = mstate->zone.size;
}

void REleHTMLCell::onCompositChildrenEnd(class IRichCompositor* compositor)
{
	m_rContentSize = m_rLineCache.flush(compositor);
	m_rMetrics.rect.extend(m_rContentSize);
}

REleHTMLCell::REleHTMLCell(class REleHTMLRow* row)
	: m_rRow(row), m_rHAlignSpecified(false), m_rVAlignSpecified(false), m_rIndexNumber(0),
	m_rHAlignment(e_align_left), m_rVAlignment(e_align_bottom)
{
	// content alignment should not effect
	m_rLineCache.setHAlign(e_align_left);
}

REleHTMLCell::~REleHTMLCell()
{
}

bool REleHTMLRow::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	m_rHAlignSpecified = parseAlignment((*attrs)["align"], m_rHAlignment);
	m_rVAlignSpecified = parseAlignment((*attrs)["valign"], m_rVAlignment);

	return true;
}

void REleHTMLRow::onCompositStatePushed(class IRichCompositor* compositor)
{
	m_rLeftWidth = m_rTable->getZoneWidth();
}

std::vector<class REleHTMLCell*>& REleHTMLRow::getCells() 
{ 
	return m_rCells; 
}

void REleHTMLRow::addChildren(IRichElement* child)
{
	REleHTMLNode::addChildren(child);

	if(dynamic_cast<class REleHTMLCell*>(child))
	{
		dynamic_cast<class REleHTMLCell*>(child)->setIndex((int)m_rCells.size());
		m_rCells.push_back(dynamic_cast<class REleHTMLCell*>(child));
	}
}

class REleHTMLTable* REleHTMLRow::getTable()
{
	return m_rTable;
}

short REleHTMLRow::getCellWidth(int index, ROptSize width)
{
	CCAssert(index < (int)m_rCells.size(), "Invalid Cell Index!");
	if ( (m_rLeftWidth == 0 && width.isZero()) || m_rCells.size() == 0 )
		return 0;

	short returned_width = 0;
	if ( width.isZero() )
	{
		returned_width =  m_rLeftWidth / (m_rCells.size() - index);
	}
	else
	{
		returned_width = width.getValueReal(m_rTable->getZoneWidth());
	}

	m_rLeftWidth -= returned_width;
	m_rLeftWidth = RMAX(0, m_rLeftWidth);
	return returned_width;
}

REleHTMLRow::REleHTMLRow(class REleHTMLTable* table)
	: m_rTable(table), m_rHAlignSpecified(false), m_rVAlignSpecified(false),
	m_rHAlignment(e_align_left), m_rVAlignment(e_align_bottom), m_rLeftWidth(0)
{

}

REleHTMLTable::EFrame REleHTMLTable::parseFrame(const std::string& str)
{
	if ( str.empty() )
		return e_box;

	if ( strcmp(str.c_str(), "void") == 0 )
	{
		return e_void;
	}
	else if ( strcmp(str.c_str(), "above") == 0 )
	{
		return e_above;
	}
	else if ( strcmp(str.c_str(), "below") == 0 )
	{
		return e_below;
	}
	else if ( strcmp(str.c_str(), "hsides") == 0 )
	{
		return e_hsides;
	}
	else if ( strcmp(str.c_str(), "lhs") == 0 )
	{
		return e_lhs;
	}
	else if ( strcmp(str.c_str(), "rhs") == 0 )
	{
		return e_rhs;
	}
	else if ( strcmp(str.c_str(), "vsides") == 0 )
	{
		return e_vsides;
	}
	else if ( strcmp(str.c_str(), "box") == 0 )
	{
		return e_box;
	}
	else if ( strcmp(str.c_str(), "border") == 0 )
	{
		return e_border;
	}

	return e_box;
}

REleHTMLTable::ERules REleHTMLTable::parseRules(const std::string& str)
{
	if ( str.empty() )
		return e_all;

	if ( strcmp(str.c_str(), "none") == 0 )
	{
		return e_none;
	}
	else if ( strcmp(str.c_str(), "groups") == 0 )
	{
		return e_groups;
	}
	else if ( strcmp(str.c_str(), "rows") == 0 )
	{
		return e_rows;
	}
	else if ( strcmp(str.c_str(), "cols") == 0 )
	{
		return e_cols;
	}
	else if ( strcmp(str.c_str(), "all") == 0 )
	{
		return e_all;
	}

	return e_all;
}


bool REleHTMLTable::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	m_rWidth = parseOptSize((*attrs)["width"]);


	m_rBorder = hasAttribute(attrs, "border") ?
		parsePixel( (*attrs)["border"] ) : 0;

	short padding =  hasAttribute(attrs, "cellpadding") ?
		parsePixel((*attrs)["cellpadding"]) : 0;
	short spacing = hasAttribute(attrs, "cellspacing") ?
		parsePixel((*attrs)["cellspacing"]) : 0;

	// color
	m_rColor = parseColor((*attrs)["bgcolor"]);
	m_rBorderColor = hasAttribute(attrs, "bordercolor") ?
		parseColor((*attrs)["bordercolor"]) : m_rBorderColor;

	// draw border
	m_rFrame = hasAttribute(attrs, "frame") ?
		parseFrame((*attrs)["frame"]) : e_void;

	m_rRules = hasAttribute(attrs, "rules") ?
		parseRules((*attrs)["rules"]) : e_none;

	m_rHAlignSpecified = parseAlignment((*attrs)["align"], m_rHAlign); // not cell alignment!

	m_rTableCache.setPadding(padding);
	m_rTableCache.setSpacing(spacing);

	m_rDirty = true;

	return true;
}

void REleHTMLTable::onCachedCompositBegin(class ICompositCache* cache, RPos& pen)
{
	if ( m_rHAlignSpecified )
	{
		m_rTempAlign = cache->getHAlign();
		cache->setHAlign(m_rHAlign);
	}
}

void REleHTMLTable::onCachedCompositEnd(class ICompositCache* cache, RPos& pen)
{
	if ( m_rHAlignSpecified )
		cache->setHAlign(m_rTempAlign);
}

void REleHTMLTable::addChildren(IRichElement* child)
{
	REleHTMLNode::addChildren(child);

	if(dynamic_cast<class REleHTMLRow*>(child))
		m_rRows.push_back(dynamic_cast<class REleHTMLRow*>(child));
}

void REleHTMLTable::onCompositStatePushed(class IRichCompositor* compositor)
{
	RMetricsState* mstate = compositor->getMetricsState();
	mstate->elements_cache = &m_rTableCache;

	mstate->zone.size.w = m_rWidth.getValueReal(mstate->zone.size.w);
	mstate->zone.size.h = 0;

	// except border, spacing, padding
	size_t cols =  m_rRows.empty() ? 0 : m_rRows[0]->getCells().size();
	short except_width = (cols - 1) * m_rTableCache.getSpacing() + cols * m_rTableCache.getPadding() * 2 + m_rBorder * 2;
	mstate->zone.size.w -= except_width;

	mstate->zone.size.w = RMAX(0, mstate->zone.size.w);
	mstate->zone.size.h = RMAX(0, mstate->zone.size.h);

	m_rZoneWidth = mstate->zone.size.w;
}

void REleHTMLTable::onCompositChildrenEnd(class IRichCompositor* compositor)
{
	RRect rect = m_rTableCache.flush(compositor);
	m_rMetrics.rect.extend(rect);
}

bool REleHTMLTable::onCompositFinish(class IRichCompositor* compositor) 
{
	m_rMetrics.rect.pos.y = m_rMetrics.rect.size.h;
	m_rMetrics.advance.x = m_rMetrics.rect.size.w;
	m_rMetrics.advance.y = 0;
	return true; 
}

void REleHTMLTable::drawThicknessLine(short left, short top, short right, short bottom, const ccColor4F& color)
{
	CCPoint vertices[4]={
		ccp(left,bottom),ccp(right,bottom),
		ccp(right,top),ccp(left,top),
	};

	ccDrawSolidPoly(vertices, 4, color);
}

void REleHTMLTable::createTicknessLineNode(RRichCanvas canvas, short left, short top, short right, short bottom, const ccColor4F& color)
{
	CCDrawNode* drawNode = CCDrawNode::create();

	CCPoint vertices[4]={
		ccp(left,bottom),ccp(right,bottom),
		ccp(right,top),ccp(left,top),
	};

	drawNode->drawPolygon(vertices, 4, color, 0.0f, color);
	drawNode->setZOrder(ZORDER_OVERLAY);
	canvas.root->addCCNode(drawNode);
}

void REleHTMLTable::onRenderPrev(RRichCanvas canvas) 
{
	if ( !m_rDirty )
		return;

	m_rDirty = false;

	RRect rect = m_rMetrics.rect;
	RPos gp = getGlobalPosition();
	short left = gp.x;
	short top = gp.y;// + canvas.root->getActualSize().h;
	short right = left + rect.size.w;
	short bottom = top - rect.size.h - 1;
	short spacing = m_rTableCache.getSpacing();

	// render background
	if ( m_rColor )
	{
		ccColor4B bgcolor4b = ccc4(
			m_rColor & 0xff, 
			m_rColor >> 8 & 0xff, 
			m_rColor >> 16 & 0xff,
			m_rColor >> 24 & 0xff);

		ccColor4F bgcolor4f = ccc4FFromccc4B(bgcolor4b);

		//drawThicknessLine(left, top, right, bottom, bgcolor4f);
		createTicknessLineNode(canvas, left, top, right, bottom, bgcolor4f);
	}

	// frame color
	ccColor4B color4b = ccc4(
		m_rBorderColor & 0xff, 
		m_rBorderColor >> 8 & 0xff, 
		m_rBorderColor >> 16 & 0xff,
		m_rBorderColor >> 24 & 0xff);

	ccColor4F color4f = ccc4FFromccc4B(color4b);

	// render frame lines
	if ( m_rBorder > 0 && m_rBorderColor )
	{
		bool draw_top = false;
		bool draw_bottom = false;
		bool draw_left = false;
		bool draw_right = false;

		switch(m_rFrame)
		{
		case e_void:
			break;
		case e_above:
			draw_top = true;
			break;
		case e_below:
			draw_bottom = true;
			break;
		case e_hsides:
			draw_top = true;
			draw_bottom = true;
			break;
		case e_lhs:
			draw_left = true;
			break;
		case e_rhs:
			draw_right = true;
			break;
		case e_vsides:
			draw_left = true;
			draw_right = true;
			break;
		case e_box:
		case e_border:
			draw_top = true;
			draw_bottom = true;
			draw_left = true;
			draw_right = true;
			break;
		}

		// top line
		if(draw_top)
			createTicknessLineNode(canvas, left, top, right, top - m_rBorder, color4f);
			//drawThicknessLine(left, top, right, top - m_rBorder, color4f);
		// bottom line
		if(draw_bottom)
			createTicknessLineNode(canvas, left, bottom + m_rBorder, right, bottom, color4f);
			//drawThicknessLine(left, bottom + m_rBorder, right, bottom, color4f);
		// left line
		if(draw_left)
			createTicknessLineNode(canvas, left, top, left + m_rBorder, bottom, color4f);
			//drawThicknessLine(left, top, left + m_rBorder, bottom, color4f);
		// right line
		if(draw_right)
			createTicknessLineNode(canvas, right - m_rBorder, top, right, bottom, color4f);
			//drawThicknessLine(right - m_rBorder, top, right, bottom, color4f);
	}

	bool draw_hline = false;
	bool draw_vline = false;
	switch(m_rRules)
	{
	case e_none:
	case e_groups:
		break;
	case e_rows:
		draw_hline = true;
		break;
	case e_cols:
		draw_vline = true;
		break;
	case e_all:
		draw_hline = true;
		draw_vline = true;
		break;
	};

	// render rows
	if (draw_hline)
	{
		for ( size_t i = 1; i < m_rRows.size(); i++ )
		{
			short pen_y = m_rRows[i]->getLocalPosition().y;
			short rleft = left; 
			short rright = right;
			short rtop = top + pen_y + spacing;
			short rbottom = rtop - spacing;

			CCPoint vertices[4] = {
				ccp(rleft,rbottom),ccp(rright,rbottom),
				ccp(rright,rtop),ccp(rleft,rtop)
			};

			//drawThicknessLine(rleft, rtop, rright, rbottom, color4f);
			createTicknessLineNode(canvas, rleft, rtop, rright, rbottom, color4f);
		}
	}

	// render cols
	if ( draw_vline && m_rRows.size() > 0 )
	{
		std::vector<class REleHTMLCell*>& cells = m_rRows[0]->getCells();

		for( size_t i = 1; i < cells.size(); i++ )
		{
			short pen_x = cells[i]->getLocalPosition().x;
			short cleft = left + m_rBorder + pen_x - spacing; 
			short cright = left + m_rBorder + pen_x;
			short ctop = top;
			short cbottom = bottom;

			CCPoint vertices[4] = {
				ccp(cleft,cbottom),ccp(cright,cbottom),
				ccp(cright,ctop),ccp(cleft,ctop)
			};

			//drawThicknessLine(cleft, ctop, cright, cbottom, color4f);
			createTicknessLineNode(canvas, cleft, ctop, cright, cbottom, color4f);
		}
	}
}

REleHTMLTable::REleHTMLTable()
: m_rBorderColor(0xff000000) 
, m_rFrame(e_box)
, m_rRules(e_all)
, m_rHAlignSpecified(false)
, m_rHAlign(e_align_left)
, m_rTempAlign(e_align_left)
, m_rZoneWidth(0)
{
	m_rTableCache.setTable(this);
}


//////////////////////////////////////////////////////////////////////////
// HTML CCRich Extensions

bool REleHTMLImg::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	m_filename = (*attrs)["src"];
	m_alt = (*attrs)["alt"];

	if ( hasAttribute(attrs, "texture-rect") )
	{
		RMargin margin = REleHTMLNode::parseMargin( (*attrs)["texture-rect"] );

		m_rTexture.rect.pos.x = margin.left;
		m_rTexture.rect.pos.y = margin.top;
		m_rTexture.rect.size.h = margin.bottom - margin.top;
		m_rTexture.rect.size.w = margin.right - margin.left;
	}

	return true;
}

void REleHTMLImg::onCompositStart(class IRichCompositor* compositor)
{
	CCTexture2D* texture = cocos2d::CCTextureCache::sharedTextureCache()->addImage(m_filename.c_str());

	if ( texture )
	{
		m_rTexture.setTexture(texture);

		if ( m_rTexture.rect.size.w == 0 )
		{
			m_rTexture.rect.size.w = texture->getPixelsWide();
		}
		if ( m_rTexture.rect.size.h == 0 )
		{
			m_rTexture.rect.size.h = texture->getPixelsHigh();
		}

		if ( m_rMetrics.rect.size.w == 0 )
		{
			m_rMetrics.rect.size.w = m_rTexture.rect.size.w;
		}

		if ( m_rMetrics.rect.size.h == 0 )
		{
			m_rMetrics.rect.size.h = m_rTexture.rect.size.h;
		}

		m_rMetrics.advance.x = m_rMetrics.rect.pos.x + m_rMetrics.rect.size.w;
		m_rMetrics.advance.y = 0;

		m_rMetrics.rect.pos.y = m_rTexture.rect.size.h;
	}
}

bool REleHTMLTouchable::isLocationInside(CCPoint location)
{
	RPos ele_pos = getGlobalPosition();
	ele_pos.sub(getLocalPosition()); // correct the position

	for ( std::list<RRect>::iterator it = m_rSpans.begin(); it != m_rSpans.end(); it++ )
	{
		RRect local_rect = *it;
		local_rect.pos.add(ele_pos);

		CCRect rect;
		rect.origin.setPoint(local_rect.pos.x, local_rect.min_y());
		rect.size.setSize(local_rect.size.w, local_rect.size.h);

		if ( rect.containsPoint(location) )
		{
			return true;
		}
	}

	return false;
}


// touch events
bool REleHTMLTouchable::onTouchBegan(CCNode* container, CCTouch *touch, CCEvent *evt)
{
	return true;
}
void REleHTMLTouchable::onTouchMoved(CCNode* container, CCTouch *touch, CCEvent *evt)
{
	CCPoint pt = container->convertToNodeSpace(touch->getLocation());
	
}
void REleHTMLTouchable::onTouchEnded(CCNode* container, CCTouch *touch, CCEvent *evt)
{
	CCPoint pt = container->convertToNodeSpace(touch->getLocation());
}
void REleHTMLTouchable::onTouchCancelled(CCNode* container, CCTouch *touch, CCEvent *evt)
{
	CCPoint pt = container->convertToNodeSpace(touch->getLocation());
	
}

void REleHTMLTouchable::onCompositStart(class IRichCompositor* compositor)
{
	compositor->getContainer()->addOverlay(this);
}

REleHTMLTouchable::REleHTMLTouchable()
	: m_rEnabled(false)
{

}

bool REleHTMLButton::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	unsigned int color = 0;

	m_rName = (*attrs)["name"];
	m_rValue = (*attrs)["value"];

	color = REleHTMLNode::parseColor((*attrs)["bgcolor"]);

	setDrawUnderline(true);
	setDrawBackground(false);
	if ( color )
	{
		setDrawBackground(true);
		m_rBGColor = color;
	}

	setEnabled(true);

	return true;
}


void REleHTMLButton::onTouchEnded(CCNode* container, CCTouch *touch, CCEvent *evt)
{
	REleHTMLTouchable::onTouchEnded(container, touch, evt);
	CCLog("[Rich HTML Button Clicked] name=%s, value=%s", m_rName.c_str(), m_rValue.c_str());
}


bool REleHTMLAnchor::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	unsigned int color = 0;

	m_rName = (*attrs)["name"];
	m_rHref = (*attrs)["href"];

	color = REleHTMLNode::parseColor((*attrs)["bgcolor"]);

	setDrawUnderline(true);
	setDrawBackground(false);
	if ( color )
	{
		setDrawBackground(true);
		m_rBGColor = color;
	}

	setEnabled(true);

	return true;
}


static REleCCBNode::ccb_reader_t s_ccb_reader = NULL;
void REleCCBNode::registerCCBReader(ccb_reader_t reader)
{
	s_ccb_reader = reader;
}

bool REleCCBNode::onParseAttributes(class IRichParser* parser, attrs_t* attrs )
{
	unsigned int color = 0;

	m_filename = (*attrs)["src"];
	
	if ( !m_filename.empty() )
	{
		if ( s_ccb_reader )
		{
			m_ccbNode = s_ccb_reader(m_filename.c_str());
		}
		else
		{
			cocos2d::extension::CCNodeLoaderLibrary * ccNodeLoaderLibrary = cocos2d::extension::CCNodeLoaderLibrary::newDefaultCCNodeLoaderLibrary();
			cocos2d::extension::CCBReader * ccbReader = new cocos2d::extension::CCBReader(ccNodeLoaderLibrary);
			m_ccbNode = ccbReader->readNodeGraphFromFile(m_filename.c_str(), NULL);
			ccbReader->release();
		}

		if ( m_ccbNode )
		{
			m_ccbNode->retain();
			m_ccbNode->setAnchorPoint(ccp(0.0f, 1.0f));
			m_ccbNode->ignoreAnchorPointForPosition(true);
			m_rMetrics.rect.size.w = (short)m_ccbNode->getContentSize().width;
			m_rMetrics.rect.size.h = (short)m_ccbNode->getContentSize().height;
			m_rMetrics.advance.x = m_rMetrics.rect.size.w;
			m_rMetrics.rect.pos.y = m_rMetrics.rect.size.w;
			m_dirty = true;

			CCBAnimationManager* anim_manager = dynamic_cast<CCBAnimationManager*>(m_ccbNode->getUserObject());
			if ( anim_manager && strcmp((*attrs)["play"].c_str(), "auto") == 0 )
			{
				m_sequence = (*attrs)["anim"];
				if ( !m_sequence.empty() )
					anim_manager->runAnimations(m_sequence.c_str());
			}

			return true;
		}
	}

	return false;
}

bool REleCCBNode::onCompositFinish(class IRichCompositor* compositor) 
{
	return true;
}

void REleCCBNode::onRenderPost(RRichCanvas canvas)
{
	if ( m_dirty )
	{
		RPos pos = getGlobalPosition();
		m_ccbNode->setPosition(ccp(pos.x, pos.y - m_rMetrics.rect.size.h /*+ canvas.root->getActualSize().h*/));
		canvas.root->addCCNode(m_ccbNode);
		m_dirty = false;
	}

	REleBase::onRenderPost(canvas);
}

REleCCBNode::REleCCBNode()
	: m_ccbNode(NULL), m_dirty(false)
{

}

REleCCBNode::~REleCCBNode()
{
	CC_SAFE_RELEASE(m_ccbNode);
}


NS_CC_EXT_END;

