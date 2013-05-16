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
#include "dfont_manager.h"
#include "dfont_utility.h"

#include <cocos2d.h>

#include <fstream>

using namespace cocos2d;

namespace dfont
{


void GlyphSlot::retain()
{
	++ref_count;
	if ( ref_count == 1 )
	{
		texture->_slot_inuse(this);
	}
}
void GlyphSlot::release()
{
	--ref_count;
	if ( ref_count == 0 )
	{
		texture->_slot_nouse(this);
	}
}


WTexture2D::WTexture2D(FontInfo* f, int width, int height, int padding_width, int padding_height)
	: m_font(f), m_width(width), m_height(height), 
	m_padding_width(padding_width), m_padding_height(padding_height),
	m_data(NULL), m_user_texture(NULL)
{
	m_cols = m_width/m_padding_width;
	m_rows = m_height/m_padding_height;
	m_slot_num = m_cols * m_rows;
	m_slots = new GlyphSlot[m_slot_num];

	for ( size_t i = 0; i < m_slot_num; i++ )
	{
		_init_slot(i);
	}

#if _DFONT_DEBUG
	m_data = new unsigned char[width * height * 4];
	memset(m_data, 0, width * height * 4);

	for( int i = 0; i < width * height * 4; i+=4 )
	{
		int row = (i/4) / width;
		int col = (i/4) % width;
		if (col % m_padding_width == 0 || row % m_padding_height == 0)
		{
			//int* p = &m_data[i];
			*(int*)(&m_data[i]) = 0xffff0000;
		}
		else
		{
			*(int*)(&m_data[i]) = 0x80008000;
		}
	}
#endif//_DFONT_DEBUG

	CCTexture2D* tex = new CCTexture2D;
	tex->initWithData(m_data, kCCTexture2DPixelFormat_RGBA8888, m_width, m_height, CCSize(m_width, m_height));

	ccTexParams tparam;
	tparam.magFilter = GL_NEAREST;
	tparam.minFilter = GL_NEAREST;
	tparam.wrapS = GL_CLAMP_TO_EDGE;
	tparam.wrapT = GL_CLAMP_TO_EDGE;
	tex->setTexParameters(&tparam);
	
	m_user_texture = tex;
}

WTexture2D::~WTexture2D()
{
	m_dirty_slots.clear();
	if ( m_user_texture )
	{
		user_texture<CCTexture2D>()->release();
		m_user_texture = NULL;
	}
	delete[] m_slots;
	delete[] m_data;
}

int WTexture2D::width()
{
	return m_width;
}
int WTexture2D::height()
{
	return m_height;
}

// flush data to GPU
void WTexture2D::flush()
{
	ccGLBindTexture2D(user_texture<CCTexture2D>()->getName());

	for ( size_t i = 0; i < m_dirty_slots.size(); i++ )
	{
		GlyphSlot* slot = m_dirty_slots[i];		

		glTexSubImage2D(GL_TEXTURE_2D, 0, 
			slot->padding_rect.origin_x, slot->padding_rect.origin_y,
			slot->metrics.width, slot->metrics.height,
			GL_RGBA, GL_UNSIGNED_BYTE, slot->bitmap->get_buffer()
			);

		slot->bitmap->release();
		slot->bitmap = NULL;
	}

	m_dirty_slots.clear();
}

GlyphSlot* WTexture2D::cache_charcode(utf32 charcode)
{
	GlyphSlot* slot = NULL;

	if ( m_emptyslots.empty() )
	{
		return NULL;
	}
	slot = *m_emptyslots.begin();

	if ( slot )
	{
		GlyphBitmap bm;
		if ( m_font->render_charcode(charcode, &bm) )
		{
			slot->charcode = charcode;
			slot->metrics.left = bm.top_left_pixels.x;
			slot->metrics.top = bm.top_left_pixels.y;
			slot->metrics.width = bm.bitmap->width();
			slot->metrics.height = bm.bitmap->height();
			slot->metrics.advance_x = bm.advance_pixels.x;
			slot->metrics.advance_y = bm.advance_pixels.y;
			slot->bitmap = bm.bitmap;

#if _DFONT_DEBUG
			_dump2texture(bm.bitmap, slot->padding_rect, false);
#endif

			// bitmap will release later after update texture
			m_dirty_slots.push_back(slot);
		}
		else
		{
			// render a bad char!
			slot = NULL;
		}
	}

	// no more free slot yet!
	return slot;
}

unsigned char* WTexture2D::buffer_data()
{
	return m_data;
}

void WTexture2D::dump_textures(const char* prefix, int index)
{
	char path_buffer[256];
	sprintf(path_buffer, "%sdfont_%s_%2d.tga", CCFileUtils::sharedFileUtils()->getWritablePath().c_str(), prefix, index);

#if	_DFONT_DEBUG
	if ( m_data )
	{
		dump2tga(path_buffer, (unsigned int*)m_data, this->width(), this->height());
	}
#endif//_DFONT_DEBUG
}

void WTexture2D::_init_slot(int i)
{
	GlyphSlot& slot = m_slots[i];
	slot.charcode = 0;
	slot.ref_count = 0;
	slot.texture = this;
	slot.padding_rect.width = m_padding_width;
	slot.padding_rect.height = m_padding_height;
	slot.padding_rect.origin_x = i % m_cols * m_padding_width;
	slot.padding_rect.origin_y = i / m_cols * m_padding_height;

	m_emptyslots.insert(&slot);
}

void WTexture2D::_slot_nouse(GlyphSlot* slot)
{
	m_emptyslots.insert(slot);
}

void WTexture2D::_slot_inuse(GlyphSlot* slot)
{
	std::set<GlyphSlot*>::iterator it = m_emptyslots.find(slot);
	if(it != m_emptyslots.end())
	{
		m_emptyslots.erase(it);
	}
}

void WTexture2D::_dump2texture(IBitmap* bitmap, const PaddingRect& rect, bool draw_cbox/* = false*/)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = rect.origin_x + bitmap->width();
	FT_Int  y_max = rect.origin_y + bitmap->height();

	for ( i = rect.origin_x, p = 0; i < x_max; i++, p++ )
	{
		for ( j = rect.origin_y, q = 0; j < y_max; j++, q++ )
		{
			if ( i < 0 || j < 0 || i >= m_width || j >= m_height )
				continue;

			int pos = j * m_width * 4 + i * 4;
			ColorRGBA src = bitmap->get_unit_at(p, q);
			ColorRGBA bgc(m_data[pos + 0], m_data[pos + 1 ], m_data[pos + 2 ], m_data[pos + 3 ]);
			ReplaceBlender blender;
			ColorRGBA color = blender.blend(src, bgc);
			//if ( color.a )
			{
				m_data[pos + 0 ] = color.r;
				m_data[pos + 1 ] = color.g;
				m_data[pos + 2 ] = color.b;
				m_data[pos + 3 ] = color.a;
			}

			// draw cbox
			if (draw_cbox)
				if ( p == 0 || q == 0 || p == bitmap->width() - 1 || q == bitmap->height() - 1 )
				{
					m_data[pos + 0 ] = 0x00;
					m_data[pos + 1 ] = 0xff;
					m_data[pos + 2 ] = 0x00;
					m_data[pos + 3 ] = 0xff;
				}
		}
	}
}

void FontCatalog::require_text(utf16* text, size_t len, std::vector<GlyphSlot*>* glyph_slots)
{
	for ( size_t i = 0; i < len; i++ )
	{
		GlyphSlot* slot = NULL;

		if ( cocos2d::isspace_unicode(text[i]) )
		{
			slot = require_char(c_char_blank);
		}
		else
		{
			slot = require_char(text[i]);
		}

		if ( !slot )
		{
			slot = require_char(c_char_invalid);
		}

		if ( slot )
		{
			glyph_slots->push_back(slot);
		}
	}
	this->flush();
}

void FontCatalog::require_text(utf32* text, size_t len, std::vector<GlyphSlot*>* glyph_slots)
{
	for ( size_t i = 0; i < len; i++ )
	{
		GlyphSlot* slot = NULL;

		if ( cocos2d::isspace_unicode((utf16)text[i]) )
		{
			slot = require_char(c_char_blank);
		}
		else
		{
			slot = require_char(text[i]);
		}

		if ( !slot )
		{
			slot = require_char(c_char_invalid);
		}

		if ( slot )
		{
			glyph_slots->push_back(slot);
		}
	}
	this->flush();
}

GlyphSlot* FontCatalog::require_char(utf32 charcode)
{
	GlyphSlot* slot = NULL;

	// find if already created
	glyph_map_t::iterator it = m_glyphmap.find(charcode);
	if ( it != m_glyphmap.end() )
	{
		slot = it->second;
	}
	else
	{
		//
		// create a new char
		//

		for ( size_t i = 0; i < m_textures.size(); i++ )
		{
			slot = m_textures[i]->cache_charcode(charcode);
			if ( slot )
			{
				break;
			}
		}

		// no more empty slots, create a new texture
		if (!slot && (int)m_textures.size() < m_max_textures)
		{
			WTexture2D* newtex = new WTexture2D(m_font, m_texture_width, m_texture_height, m_padding_width, m_padding_height);
			m_textures.push_back(newtex);
			slot = newtex->cache_charcode(charcode);
		}

		if ( slot )
		{
			_remove_from_map(slot);// remove previous slot map
			_add_to_map(slot);
		}
	}

	if ( slot )
	{
		slot->retain();
	}

	return slot;
}

FontInfo* FontCatalog::font()
{
	return m_font;
}

std::vector<WTexture2D*>* FontCatalog::textures()
{
	return &m_textures;
}

void FontCatalog::flush()
{
	for ( size_t i = 0; i < m_textures.size(); i++ )
	{
		m_textures[i]->flush();
	}

	m_previous_char_idx = 0;
}

void FontCatalog::dump_textures(const char* prefix)
{
	for ( size_t i = 0; i < m_textures.size(); i++ )
	{
		m_textures[i]->dump_textures(prefix, i);
	}
}

FontCatalog::FontCatalog(FontInfo* f, int texture_width, int texture_height, int max_textures/*=2*/)
	: m_font(f), 
	m_texture_width(texture_width), 
	m_texture_height(texture_height), 
	m_max_textures(max_textures),
	m_previous_char_idx(0)
{
	int font_size = (int)(f->char_width_pt() > f->char_height_pt() ? f->char_width_pt() : f->char_height_pt());
	font_size += f->extend_pt();
	m_padding_width = font_size;
	m_padding_height = font_size;
}

FontCatalog::~FontCatalog()
{
	m_glyphmap.clear();
	m_reverse_glyphmap.clear();
	for ( size_t i = 0; i < m_textures.size(); i++ )
	{
		delete m_textures[i];
	}
	m_textures.clear();
	m_font->release();
}

void FontCatalog::_add_to_map(GlyphSlot* slot)
{
	m_glyphmap[slot->charcode] = slot;
	m_reverse_glyphmap[slot] = slot->charcode;
}

void FontCatalog::_remove_from_map(GlyphSlot* slot)
{
	reverse_glyph_map_t::iterator it = m_reverse_glyphmap.find(slot);
	if ( it != m_reverse_glyphmap.end() )
	{
		m_glyphmap.erase(it->second);
		m_reverse_glyphmap.erase(it);
	}
}


FontCatalog* FontFactory::find_font(const char* alias, bool no_fail /*= true*/)
{
	if ( !alias )
		alias = DFONT_DEFAULT_FONTALIAS;
	
	std::map<std::string, FontCatalog*>::iterator it = m_fonts.find(alias);
	if ( it != m_fonts.end() )
	{
		return it->second;
	}

	if ( no_fail )
		return m_fonts[DFONT_DEFAULT_FONTALIAS];

	return NULL;
}

FontCatalog* FontFactory::another_alias(const char* another_alias, const char* origin_alias)
{
	FontCatalog* fontc = find_font(origin_alias);

	if ( fontc )
	{
		m_fonts[another_alias] = fontc;
	}

	return fontc;
}

FontCatalog* FontFactory::create_font(
	const char* alias, const char* font_name, int color, int size_pt,
	EFontStyle style/*=e_plain*/, float strength/*=1.0f*/, int secondary_color/*=0xff000000*/, 
	int faceidx/*=0*/, int ppi/*=DFONT_DEFAULT_FONTPPI*/
	)
{
	if ( !alias )
	{
		return NULL;
	}
	FontCatalog* catalog = find_font(alias, false);
	FontInfo* font = NULL;
	if ( catalog )
	{
		// the alias is already in used, return it.
		return catalog;
	}

	std::string fullpath = CCFileUtils::sharedFileUtils()->fullPathForFilename(font_name);

	font = FontInfo::create_font(m_library, fullpath.c_str(), faceidx, size_pt, size_pt, ppi);
	if ( !font )
	{
		return find_font(DFONT_DEFAULT_FONTALIAS);
	}

	switch (style)
	{
	case e_plain:
		font->add_pass(RenderPassParam(color, e_replace_blender, 0, 0, false, 0));
		break;
	case e_strengthen:
		font->add_pass(RenderPassParam(color, e_replace_blender, 0, 0, true, (FT_F26Dot6)strength*64));
		break;
	case e_border:
		font->add_pass(RenderPassParam(secondary_color, e_replace_blender, 0, 0, true, (FT_F26Dot6)strength*64))
			->add_pass(RenderPassParam(color, e_additive_blender, 0, 0, false, 0));
		break;
	case e_shadow:
		font->add_pass(RenderPassParam(secondary_color, e_replace_blender, (int)strength, (int)-strength, false, secondary_color))
			->add_pass(RenderPassParam(color, e_additive_blender, 0, 0, false, 0));
		break;
	}

	catalog = new FontCatalog(font, 
		DFONT_TEXTURE_SIZE_WIDTH, 
		DFONT_TEXTURE_SIZE_HEIGHT, 
		DFONT_MAX_TEXTURE_NUM_PERFONT);

	m_fonts[alias] = catalog;

	return catalog;
}

void FontFactory::dump_textures()
{
	std::map<std::string, FontCatalog*>::iterator it = m_fonts.begin();
	for ( ; it != m_fonts.end(); it++ )
	{
		it->second->dump_textures(it->first.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
static FontFactory::initor_t s_initor = NULL;

void FontFactory::register_initor(initor_t initor)
{
	s_initor = initor;
}

FontFactory* FontFactory::instance()
{
	static FontFactory* _factory = NULL;
	if ( _factory == NULL )
	{
		// lazy init
		_factory = new FontFactory;

		if ( s_initor )
		{
			s_initor();
		}
		else
		{
			dfont_default_initialize();
		}
	}
	return _factory;
}


FontFactory::FontFactory()
{
	FT_Init_FreeType(&m_library);
}

FontFactory::~FontFactory()
{
	std::set<FontCatalog*> delset;
	std::map<std::string, FontCatalog*>::iterator it = m_fonts.begin();
	for ( ; it != m_fonts.end(); it++ )
	{
		if ( delset.find(it->second) == delset.end() )
		{
			delete it->second;
			delset.insert(it->second);
		}
	}
	m_fonts.clear();
	FT_Done_FreeType(m_library);
}

}//namespace dfont

