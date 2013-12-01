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
#ifndef __DFONT_MANAGER_H__ 
#define __DFONT_MANAGER_H__

#include "dfont_config.h"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <deque>

namespace dfont
{

typedef unsigned long utf32;
typedef unsigned short utf16;
typedef unsigned char utf8;

const utf32 c_char_invalid = '*';
const utf32 c_char_blank = ' ';

enum EFontStyle
{
	e_plain,
	e_strengthen,
	e_border,
	e_shadow
};

struct PaddingRect
{
	int origin_x;
	int origin_y;
	int width;
	int height;
};

struct GlyphMetrics
{
	int left;
	int top;
	int width;
	int height;
	int advance_x;
	int advance_y;
};

struct GlyphSlot
{
	utf32 charcode; // unicode
	size_t ref_count;	// counter for using
	PaddingRect padding_rect;
	GlyphMetrics metrics;
	class WTexture2D* texture;

	class IBitmap* bitmap;

	void retain();
	void release();
};

// represent a 2d texture
class WTexture2D
{
	friend struct GlyphSlot;
public:
	WTexture2D(class FontInfo* f, int width, int height, int padding_width, int padding_height);
	~WTexture2D();

	int width();
	int height();

	// flush data to GPU
	void flush();

	bool has_empty_slot();

	GlyphSlot* cache_charcode(utf32 charcode);

	unsigned char* buffer_data();

	// T == cocos2d::CCTexture2D
	template<typename T>
	T* user_texture()
	{
		return (T*)m_user_texture;
	}

	void dump_textures(const char* prefix, int index);

private:
	void _init_slot(int i);

	void _slot_nouse(GlyphSlot* slot);

	void _slot_inuse(GlyphSlot* slot);

	void _dump2texture(class IBitmap* bitmap, const PaddingRect& rect, bool draw_cbox = false);


	class FontInfo* m_font;
	GlyphSlot* m_slots;
	size_t m_slot_num;

	int m_width;
	int m_height;
	int m_cols;
	int m_rows;
	int m_padding_width;
	int m_padding_height;

	unsigned char* m_data;
	void* m_user_texture;
	std::vector<GlyphSlot*> m_dirty_slots;

	std::set<GlyphSlot*> m_emptyslots;
};

class FontCatalog
{
public:
	typedef std::map<utf32,  GlyphSlot*> glyph_map_t;
	typedef std::map<GlyphSlot*, utf32> reverse_glyph_map_t;

	void require_text(utf16* text, size_t len, std::vector<GlyphSlot*>* glyph_slots);
	void require_text(utf32* text, size_t len, std::vector<GlyphSlot*>* glyph_slots);
	GlyphSlot* require_char(utf32 charcode);
	//class FontInfo* font();
	std::vector<WTexture2D*>* textures();
	void flush();

	// utilities
	unsigned int char_width();
	unsigned int char_height();

	bool add_hackfont(const char* fontname, std::set<unsigned long>* charset, unsigned int shift_y = 0);
	bool add_hackfont(const char* fontname, long face_idx, std::set<unsigned long>* charset, unsigned int shift_y);

	void dump_textures(const char* prefix);

	FontCatalog(class FontInfo* f, int texture_width, int texture_height, int max_textures=2);

	~FontCatalog();

private:
	void _add_to_map(GlyphSlot* slot);

	void _remove_from_map(GlyphSlot* slot);

	class FontInfo* m_font;
	std::vector<WTexture2D*> m_textures;
	glyph_map_t m_glyphmap;
	reverse_glyph_map_t m_reverse_glyphmap;

	int m_max_textures;

	int m_texture_width;
	int m_texture_height;
	int m_padding_width;
	int m_padding_height;

	utf32 m_previous_char_idx;
};

class FontFactory
{
public:
	typedef void (*initor_t)();
public:
	static void register_initor(initor_t initor);
	static FontFactory* instance();

	FontCatalog* find_font(const char* alias, bool no_fail = true);

	FontCatalog* create_font(
		const char* alias, const char* font_name, unsigned int color, int size_pt,
		EFontStyle style=e_plain, float strength=1.0f, unsigned int secondary_color=0xff000000, 
		int faceidx=0, int ppi=DFONT_DEFAULT_FONTPPI
		);

	FontCatalog* another_alias(const char* another_alias, const char* origin_alias);

	void dump_textures();

private:
	FontFactory();
	~FontFactory(); 

	std::map<std::string, FontCatalog*> m_fonts;
};

}

#endif//__DFONT_MANAGER_H__
