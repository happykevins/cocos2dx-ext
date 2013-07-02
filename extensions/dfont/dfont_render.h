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
#ifndef __DFONT_RENDER_H__ 
#define __DFONT_RENDER_H__

#include "dfont_config.h"

#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

namespace dfont
{
//////////////////////////////////////////////////////////////////////////
// defines

inline FT_Pos floor_26dot6(FT_Pos p)
{
	return p & ~63;
}

inline FT_Pos ceiling_26dot6(FT_Pos p)
{
	return ( p + 63 ) & ~63;
}

inline FT_Pos round_26dot6(FT_Pos p)
{
	return ( p + 32 ) & ~63;
}

inline FT_BBox align_bbox(FT_BBox& box_26dot6)
{
	box_26dot6.xMin = floor_26dot6( box_26dot6.xMin );
	box_26dot6.yMin = floor_26dot6( box_26dot6.yMin );
	box_26dot6.xMax = ceiling_26dot6( box_26dot6.xMax );
	box_26dot6.yMax = ceiling_26dot6( box_26dot6.yMax );
	return box_26dot6;
}

inline FT_BBox intersect_bbox(FT_BBox& box_26dot6, const FT_BBox& other_box_26dot6)
{
	box_26dot6.xMin = box_26dot6.xMin < other_box_26dot6.xMin ? box_26dot6.xMin : other_box_26dot6.xMin;
	box_26dot6.yMin = box_26dot6.yMin < other_box_26dot6.yMin ? box_26dot6.yMin : other_box_26dot6.yMin;
	box_26dot6.xMax = box_26dot6.xMax > other_box_26dot6.xMax ? box_26dot6.xMax : other_box_26dot6.xMax;
	box_26dot6.yMax = box_26dot6.yMax > other_box_26dot6.yMax ? box_26dot6.yMax : other_box_26dot6.yMax;
	return box_26dot6;
}

struct ColorRGBA
{
	FT_Byte r;
	FT_Byte g;
	FT_Byte b;
	FT_Byte a;

	ColorRGBA(): r(0), g(0), b(0), a(0) {}
	ColorRGBA(const FT_UInt32& rgba) { *(int*)this = rgba; }
	ColorRGBA(FT_Byte _r, FT_Byte _g , FT_Byte _b, FT_Byte _a): r(_r), g(_g), b(_b), a(_a) {}
	ColorRGBA(const ColorRGBA& other) { r = other.r; g = other.g; b = other.b; a = other.a; }
	inline FT_UInt32 to_uint32() const { return *(int*)this; }
};

//////////////////////////////////////////////////////////////////////////
// bitmap

class IBitmap
{
public:
	virtual ~IBitmap(){}
	virtual void release() = 0;
	virtual int width() = 0;
	virtual int height() = 0;
	virtual int padding() = 0;
	virtual int real_width() = 0;
	virtual int real_height() = 0;
	virtual int numbits() = 0;
	virtual const void* get_buffer() = 0;
	virtual bool is_managed() = 0;
	virtual unsigned int get_unit_at(int pos_x, int pos_y) = 0;
	virtual void set_unit_at(unsigned int data, int pos_x, int pos_y) = 0;
	virtual bool check_contains(int pos_x, int pos_y) = 0;
};

class Bitmap_32bits: public IBitmap
{
public:
	Bitmap_32bits(int real_width, int real_height, int padding=0);
	Bitmap_32bits(unsigned int* buf, int real_width, int real_height, int padding=0);
	virtual ~Bitmap_32bits();
	virtual void release();
	
	virtual int width() { return m_width - m_padding*2; }
	virtual int height() { return m_height - m_padding*2; }
	virtual int padding() { return m_padding; }
	virtual int real_width() { return m_width; }
	virtual int real_height() { return m_height; }
	virtual int numbits() { return sizeof(unsigned int) << 3; }
	virtual bool is_managed() { return m_managed; }
	virtual const void* get_buffer() { return m_buffer; }

	virtual unsigned int get_unit_at(int pos_x, int pos_y)
	{
		pos_x += m_padding; pos_y += m_padding;
		return m_buffer[pos_y*m_width + pos_x];
	}

	virtual void set_unit_at(unsigned int data, int pos_x, int pos_y)
	{
		pos_x += m_padding; pos_y += m_padding;
		m_buffer[pos_y*m_width + pos_x] = (unsigned int)(data & (unsigned int)-1);
	}

	virtual bool check_contains(int pos_x, int pos_y)
	{
		pos_x += m_padding; pos_y += m_padding;
		if ( pos_x < 0 || pos_x >= m_width || pos_y < 0 || pos_y >= m_height )
			return false;

		return true;
	}

private:
	unsigned int* m_buffer;
	int m_width;
	int m_height;
	int m_padding;
	bool m_managed;
};

struct GlyphBitmap
{
	IBitmap* bitmap;
	FT_Vector top_left_pixels;
	FT_Vector advance_pixels;
	FT_Vector kerning_pixels;
	GlyphBitmap() : bitmap(NULL), top_left_pixels(), advance_pixels(), kerning_pixels() {}
};

//////////////////////////////////////////////////////////////////////////
// blender

enum EBlenderType
{
	e_replace_blender,
	e_additive_blender,
	e_alpha_blender,
	e_blender_num
};

extern const class IPixelBlender* PixelBlenders[e_blender_num];

class IPixelBlender
{
public:
	virtual ~IPixelBlender(){}
	virtual ColorRGBA blend(const ColorRGBA& src, const ColorRGBA& dst) const = 0;

protected:
	static inline 
	FT_Byte _add_byte(int a1, int a2)
	{
		int v = a1 + a2;
		return v < 0xff ? (FT_Byte)v : 0xff;
	}

	static inline
	FT_Byte _mul_byte(int m1, int m2)
	{
		return (FT_Byte)(m1 * m2 / 255.0f);
	}
};

class ReplaceBlender : public IPixelBlender
{
public:
	virtual ColorRGBA blend(const ColorRGBA& src, const ColorRGBA& dst) const
	{
		return src;
	}
};

class AlphaBlender : public IPixelBlender
{
public:
	virtual ColorRGBA blend(const ColorRGBA& src, const ColorRGBA& dst) const
	{
		FT_Byte a = src.a;
		FT_Byte r = (FT_Byte) (( (int)src.r * src.a + (int)dst.r * (0xff - src.a) ) / 255.0f);
		FT_Byte g = (FT_Byte) (( (int)src.g * src.a + (int)dst.g * (0xff - src.a) ) / 255.0f);
		FT_Byte b = (FT_Byte) (( (int)src.b * src.a + (int)dst.b * (0xff - src.a) ) / 255.0f);
		return ColorRGBA(r, g, b, a);
	}
};

class AdditiveBlender : public IPixelBlender
{
public:
	virtual ColorRGBA blend(const ColorRGBA& src, const ColorRGBA& dst) const
	{
		if (dst.to_uint32() == 0)
		{
			return src;
		}
		FT_Byte a = _add_byte(src.a, dst.a);
		FT_Byte r = (dst.r + _mul_byte(src.r - dst.r, src.a));
		FT_Byte g = (dst.g + _mul_byte(src.g - dst.g, src.a));
		FT_Byte b = (dst.b + _mul_byte(src.b - dst.b, src.a));
		return ColorRGBA(r, g, b, a);
	}
};

//////////////////////////////////////////////////////////////////////////
// render pass

struct RenderPassParam
{
	ColorRGBA		color;
	EBlenderType	blender;
	int				translate_x;
	int				translate_y;
	bool			stroke;
	FT_F26Dot6		stroke_radius;	// 26.6 format

	RenderPassParam(
		ColorRGBA c, EBlenderType b, 
		int tx, int ty, 
		bool _stroke=false, FT_F26Dot6 _stroke_radius = 64);
};

class IRenderPass
{
public:
	virtual ~IRenderPass(){}
	virtual void init(const RenderPassParam& param) = 0;
	virtual FT_Error pre_render(FT_Glyph& glyph) = 0;

	// buffer cbox is the max box of all passes, so may larger than current pass cbox
	virtual FT_Error post_render(IBitmap* buf, const FT_BBox& buf_cbox) = 0;

	virtual const IPixelBlender* blender() = 0;
	virtual ColorRGBA color() = 0;
	virtual FT_Vector translate() = 0;
	virtual const FT_BBox& cbox() = 0;
	virtual bool stroke() = 0;
	virtual FT_F26Dot6 stroke_radius() = 0;
};

class BaseRenderPass : public IRenderPass
{
public:
	BaseRenderPass();
	virtual void reset();

	virtual void init(const RenderPassParam& param);

	virtual const FT_BBox& cbox();
	virtual void set_cbox(const FT_BBox& other);

	virtual ColorRGBA color();
	virtual void set_color(ColorRGBA _color);
	virtual FT_Vector translate();
	virtual void set_translate(FT_Pos pixel_x, FT_Pos pixel_y);
	virtual bool stroke();
	virtual void set_stroke(bool on);
	virtual FT_F26Dot6 stroke_radius();
	virtual void set_stroke_radius(FT_F26Dot6 radius);

	virtual const IPixelBlender* blender();
	virtual void set_blender(EBlenderType bt);

	virtual FT_Error pre_render(FT_Glyph& glyph);
	virtual FT_Error post_render(IBitmap* buf, const FT_BBox& buf_cbox);

protected:
	virtual FT_Error pre_render_impl() = 0;
	virtual FT_Error post_render_impl(IBitmap* buf, const FT_BBox& buf_cbox) = 0;

protected:
	ColorRGBA	m_color;	// render color
	FT_BBox		m_cbox;		// the control box
	FT_Glyph	m_glyph;	// the glyph to render
	FT_Vector	m_translate;	// translate
	bool		m_stroke;	// if stroke
	FT_F26Dot6	m_stroke_radius;   // stroke thickness: 26.6f
	const IPixelBlender* m_blender;// pixel blender for render
};

class BitmapRenderPass: public BaseRenderPass
{
protected:
	virtual FT_Error pre_render_impl();
	virtual FT_Error post_render_impl(IBitmap* buf, const FT_BBox& buf_cbox);

private:
	virtual void _render(IBitmap* buf,const FT_BBox& buf_cbox, FT_BitmapGlyph bitmap_glyph, bool border);
};

class OutlineRenderPass: public BaseRenderPass
{
public:
	struct RenderContext
	{
		OutlineRenderPass* pass;
		IBitmap* buf;
		const FT_BBox* buf_cbox;
	};

protected:
	virtual FT_Error decorate();
	virtual FT_Error transform();
	virtual FT_Error pre_render_impl();
	virtual FT_Error post_render_impl(IBitmap* buf, const FT_BBox& buf_cbox);

	static void spans_callback(const int y, const int count, const FT_Span * const spans, void * const user);
};

//////////////////////////////////////////////////////////////////////////
// glyph renderer

class GlyphRenderer
{
public:
	GlyphRenderer();
	~GlyphRenderer();

	GlyphRenderer* init_pass();
	GlyphRenderer* add_pass(const RenderPassParam& param);

	FT_Error render(FT_Glyph& glyph, GlyphBitmap* glyph_bitmap);
	FT_Error render(FT_Glyph& glyph, IBitmap** pbuf, FT_Vector* top_left_pixel, FT_Vector* advance_pixel);

private:
	void reset();

	std::vector<IRenderPass*> m_outline_passes; 
	std::vector<IRenderPass*> m_bitmap_passes; 
};

//////////////////////////////////////////////////////////////////////////
// font

class FontInfo
{
public:
	static FontInfo* create_font(FT_Library library, const char* fontname, FT_UInt width_pt, FT_UInt height_pt, FT_UInt ppi=72);
	static FontInfo* create_font(FT_Library library, const char* fontname, FT_Long face_idx, FT_UInt width_pt, FT_UInt height_pt, FT_UInt ppi);

public:
	void release();

	FontInfo* add_pass(const RenderPassParam& param);

	// return 0 if failed, charactor index if success
	FT_UInt render_charcode(FT_ULong char_code, GlyphBitmap* bitmap, FT_UInt prev_idx = 0);

	const char* font_name();

	FT_Library library();

	bool is_bitmap();

	FT_UInt char_width_pt();

	FT_UInt char_height_pt();

	FT_UInt ppi();

	void set_shift_y(FT_UInt sy);

	FT_UInt extend_pt();

	FT_Short underline_position();

	FT_Short underline_thickness();

	FT_Face face();

	void set_available_charset(std::set<FT_ULong>* charset);

	FontInfo* add_hackfont(const char* fontname, std::set<FT_ULong>* charset, FT_UInt shift_y = 0);

	FontInfo* add_hackfont(const char* fontname, FT_Long face_idx, std::set<FT_ULong>* charset, FT_UInt shift_y);

protected:
	// from char code to char index
	FT_UInt get_char_index(FT_ULong charcode);
	bool render_charidx(FT_UInt char_idx, GlyphBitmap* bitmap, FT_UInt prev_idx = 0);

	// has kerning info
	bool has_kerning();
	FT_Vector get_kerning(FT_UInt left_idx, FT_UInt right_idx);

	FT_Error _render_ready_char(GlyphBitmap* bitmap);

	FT_GlyphSlot current_glyph();

	bool load_glyph_from_index(FT_UInt char_idx);

	bool load_glyph_from_char(FT_ULong char_code);

	void set_renderer(GlyphRenderer* _renderer);

	GlyphRenderer* renderer();

	FT_Error init(FT_Library& lib, const char* fontname, FT_Long face_idx, FT_UInt width_pt, FT_UInt height_pt, FT_UInt ppi);

	FontInfo(FT_Library lib);
	~FontInfo();

private:
	FT_Library m_library;
	std::string m_fontname;

	bool	m_isbitmap;
	FT_UInt m_char_width;
	FT_UInt m_char_height;
	FT_UInt	m_ppi;
	FT_UInt m_shift_y;
	FT_UInt m_extend_pt;

	FT_Short m_underline_position;
	FT_Short m_underline_thickness;

	FT_Face m_face;
	bool m_has_kerning;

	GlyphRenderer* m_ob_renderer;
	GlyphRenderer* m_private_renderer;

	std::set<FT_ULong>* m_available_charset;
	std::vector<FontInfo*> m_hackfonts;
};


};//namespace

#endif//__DFONT_RENDER_H__
