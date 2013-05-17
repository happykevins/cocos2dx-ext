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
#include "dfont_render.h"

namespace dfont
{

//////////////////////////////////////////////////////////////////////////
const IPixelBlender* PixelBlenders[e_blender_num] =
{
	new ReplaceBlender,
	new AdditiveBlender,
	new AlphaBlender
};


Bitmap_32bits::Bitmap_32bits(int width, int height)
	: m_buffer(NULL), m_width(width), m_height(height), m_managed(true)
{
	m_buffer = new unsigned int[width * height];
	memset(m_buffer, 0, width * height * sizeof(unsigned int));

#if	_DFONT_DEBUG
	// for debug
	for(int i = 0; i < width * height; i++ )
	{
		int row = i / width;
		int col = i % width;
		if ( row == 0 || row == height - 1 || col == 0 || col == width - 1 )
			m_buffer[i] = 0xff804040;

		//m_buffer[i] = 0x80;
	}
#endif//_DFONT_DEBUG
}
Bitmap_32bits::Bitmap_32bits(unsigned int* buf, int width, int height)
	: m_buffer(buf), m_width(width), m_height(height), m_managed(false)
{
}
Bitmap_32bits::~Bitmap_32bits() 
{
	if ( m_managed && m_buffer )
	{
		delete m_buffer;
		m_buffer = NULL;
	}
}

void Bitmap_32bits::release()
{
	delete this;
}


//////////////////////////////////////////////////////////////////////////
RenderPassParam::RenderPassParam(
	ColorRGBA c, EBlenderType b, 
	int tx, int ty, 
	bool _stroke/*=false*/, FT_F26Dot6 _stroke_radius/* = 64*/)
	: color(c), blender(b), 
	translate_x(tx), translate_y(ty), 
	stroke(_stroke), stroke_radius(_stroke_radius)
{

}

BaseRenderPass::BaseRenderPass()
{ 
	reset(); 
}
void BaseRenderPass::reset()
{
	memset(&m_cbox, 0, sizeof(m_cbox));
	m_color = ColorRGBA(0xff, 0xff, 0xff, 0xff);
	m_glyph = NULL;
	memset(&m_translate, 0, sizeof(m_translate));
	m_stroke = false;
	m_stroke_radius = 0;
	m_blender = NULL;
}

void BaseRenderPass::init(const RenderPassParam& param)
{
	set_color(param.color);
	set_blender(param.blender);
	set_translate(param.translate_x, param.translate_y);
	set_stroke(param.stroke);
	set_stroke_radius(param.stroke_radius);
}

const FT_BBox& BaseRenderPass::cbox() 
{ 
	return m_cbox; 
}
void BaseRenderPass::set_cbox(const FT_BBox& other) 
{ 
	m_cbox = other;
}

ColorRGBA BaseRenderPass::color() 
{ 
	return m_color; 
}
void BaseRenderPass::set_color(ColorRGBA _color) 
{ 
	m_color = _color; 
}
FT_Vector BaseRenderPass::translate() 
{ 
	return m_translate; 
}
void BaseRenderPass::set_translate(FT_Pos pixel_x, FT_Pos pixel_y) 
{ 
	m_translate.x = pixel_x; m_translate.y = pixel_y; 
}
bool BaseRenderPass::stroke() 
{ 
	return m_stroke; 
}
void BaseRenderPass::set_stroke(bool on) 
{
	m_stroke = on; 
}
FT_F26Dot6 BaseRenderPass::stroke_radius() 
{ 
	return m_stroke_radius; 
}
void BaseRenderPass::set_stroke_radius(FT_F26Dot6 radius) 
{ 
	m_stroke_radius = radius; 
}

const IPixelBlender* BaseRenderPass::blender()
{
	return m_blender;
}
void BaseRenderPass::set_blender(EBlenderType bt)
{
	m_blender = PixelBlenders[bt];
}

FT_Error BaseRenderPass::pre_render(FT_Glyph& glyph)
{
	FT_Error error = FT_Glyph_Copy(glyph, &m_glyph);

	return pre_render_impl();
}

FT_Error BaseRenderPass::post_render(IBitmap* buf, const FT_BBox& buf_cbox)
{
	FT_Error error = post_render_impl(buf, buf_cbox);
	FT_Done_Glyph(m_glyph);
	m_glyph = NULL;
	return error;
}

FT_Error BitmapRenderPass::pre_render_impl() 
{
	FT_Error error = 0;

	if ( m_glyph->format != FT_GLYPH_FORMAT_BITMAP )
		return -1;

	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)m_glyph;
	m_cbox.xMin = (bitmap_glyph->left + m_translate.x) << 6;
	m_cbox.xMax = m_cbox.xMin + (bitmap_glyph->bitmap.width << 6);
	
	m_cbox.yMax = (bitmap_glyph->top + 1 + m_translate.y) << 6;
	m_cbox.yMin = m_cbox.yMax - (bitmap_glyph->bitmap.rows << 6);

	// intersect 1 pixel border
	if ( m_stroke )
	{
		m_cbox.xMin -= 1 << 6;
		m_cbox.yMin -= 1 << 6;
		m_cbox.xMax += 1 << 6;
		m_cbox.yMax += 1 << 6;
	}

	return error;
}

FT_Error BitmapRenderPass::post_render_impl(IBitmap* buf, const FT_BBox& buf_cbox)
{
	FT_Error error = 0;
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)m_glyph;

	// bitmap pass pixel-mode must be mono
	if ( bitmap_glyph->bitmap.pixel_mode != FT_PIXEL_MODE_MONO )
		return -1;

	_render(buf, buf_cbox, bitmap_glyph, stroke());

	return error;
}

void BitmapRenderPass::_render(IBitmap* buf,const FT_BBox& buf_cbox, FT_BitmapGlyph bitmap_glyph, bool border)
{
	// border test 8 directions
	static const int BORDERTEST[][2] = 
	{
		{1, 0},
		{0, 1},
		{-1, 0},
		{0, -1},
		{1, 1},
		{-1, 1},
		{-1, -1},
		{1, -1},
	};

	// center aligned
	int pen_x_start = (cbox().xMin - buf_cbox.xMin) >> 6;
	int pen_y_start = (buf_cbox.yMax - cbox().yMax) >> 6;

	// border size fix-up
	if ( stroke() )
	{
		pen_x_start += 1;
		pen_y_start += 1;
	}

	int grey_level = 0;

	for ( FT_Int y = 0, pen_y = pen_y_start; y < bitmap_glyph->bitmap.rows && pen_y < buf->height(); y++, pen_y++ )
	{
		int row_start_x = y * bitmap_glyph->bitmap.pitch;
		for ( FT_Int x = 0, pen_x = pen_x_start; x < bitmap_glyph->bitmap.width && pen_x < buf->width(); x++, pen_x++ )
		{
			grey_level = (bitmap_glyph->bitmap.buffer[row_start_x + (x>>3)]<<(x%8)) & 0x80 ? 0xff : 0x0;
			if ( grey_level == 0 )
				continue;

			ColorRGBA src = color();

			if ( !border )
			{	
				if ( grey_level )
				{
					src.a = FT_Byte(grey_level * src.a / 255);
					ColorRGBA dst(buf->get_unit_at(pen_x, pen_y));
					buf->set_unit_at(blender()->blend(src, dst).to_uint32(), pen_x, pen_y);
				}
			}
			else
			{
				for ( int i = 0; i < sizeof(BORDERTEST)/sizeof(BORDERTEST[0]); i++ )
				{
					int test_x = BORDERTEST[i][0] + x;
					int test_y = BORDERTEST[i][1] + y;

					// check pixel exist 
					if ( test_x >= 0 && test_x < bitmap_glyph->bitmap.width && test_y >= 0 && test_y < bitmap_glyph->bitmap.rows
						&& ( (bitmap_glyph->bitmap.buffer[test_y * bitmap_glyph->bitmap.pitch + (test_x>>3)]<<(test_x%8)) & 0x80 ) 
						)
						continue;

					int draw_x = BORDERTEST[i][0] + pen_x;
					int draw_y = BORDERTEST[i][1] + pen_y;

					// check out of range
					if ( buf->check_contains(draw_x, draw_y) )
					{
						ColorRGBA dst(buf->get_unit_at(draw_x, draw_y));
						buf->set_unit_at(blender()->blend(src, dst).to_uint32(), draw_x, draw_y);
					}
				}
			}
		}
	}
}


FT_Error OutlineRenderPass::decorate()
{
	FT_Error error = 0;

	if ( stroke() )
	{
		FT_Stroker stroker;
		error = FT_Stroker_New(m_glyph->library, &stroker);
		if ( error )
			return error;

		FT_Stroker_Set(stroker,
			stroke_radius(),
			FT_STROKER_LINECAP_ROUND,
			FT_STROKER_LINEJOIN_ROUND,
			0);

		error = FT_Glyph_StrokeBorder(&m_glyph, stroker, 0, 0);
		if ( error )
			return error;

		FT_Stroker_Done(stroker);

		FT_Pos stroke_pixel = round_26dot6(stroke_radius()) * 2;
	}

	return error;
}

FT_Error OutlineRenderPass::transform()
{
	FT_Outline* outline = &reinterpret_cast<FT_OutlineGlyph>(m_glyph)->outline;
	FT_Outline_Translate(outline, m_translate.x << 6, m_translate.y << 6);
	return 0;
}

FT_Error OutlineRenderPass::pre_render_impl()
{
	FT_Error error = 0;
	if ( m_glyph->format != FT_GLYPH_FORMAT_OUTLINE )
		return -1;

	error = decorate();
	if ( error )
		return error;

	error = transform();
	if ( error )
		return error;

	FT_Outline* outline =
		&reinterpret_cast<FT_OutlineGlyph>(m_glyph)->outline;
	FT_Outline_Get_CBox( outline, &m_cbox );

	return error;
}

FT_Error OutlineRenderPass::post_render_impl(IBitmap* buf, const FT_BBox& buf_cbox)
{
	FT_Error error = 0;

	RenderContext ctx;
	ctx.pass = this;
	ctx.buf = buf;
	ctx.buf_cbox = &buf_cbox;

	FT_Raster_Params params;
	memset(&params, 0, sizeof(params));
	params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
	params.gray_spans = spans_callback;
	params.user = &ctx;

	FT_Outline* outline =
		&reinterpret_cast<FT_OutlineGlyph>(m_glyph)->outline;

	error = FT_Outline_Render(m_glyph->library, outline, &params);

	return error;
}

void OutlineRenderPass::spans_callback(const int y, const int count, const FT_Span * const spans, void * const user)
{
	RenderContext* ctx = (RenderContext*)user;

	int pen_pix_x = ( ctx->buf_cbox->xMin ) >> 6;
	int draw_y = ( ctx->buf_cbox->yMax >> 6 ) - 1 - y;

	if (draw_y >= 0 && draw_y < ctx->buf->height())
		for (int i = 0; i < count; ++i) 
		{
			int draw_x_start = spans[i].x - pen_pix_x;
			for ( int j = 0; j < spans[i].len; j++ )
			{
				int draw_x = draw_x_start + j;
				if ( draw_x < 0 )
					continue;
				if ( draw_x >= ctx->buf->width() )
					break;

				ColorRGBA color;
				ColorRGBA src = ctx->pass->color();
				src.a = FT_Byte((int)spans[i].coverage * src.a / 255);
				ColorRGBA dst(ctx->buf->get_unit_at(draw_x, draw_y));
				ctx->buf->set_unit_at(ctx->pass->blender()->blend(src, dst).to_uint32(), draw_x, draw_y);
			}
		}
}


GlyphRenderer::GlyphRenderer()
{

}

GlyphRenderer::~GlyphRenderer()
{
	reset();
}

void GlyphRenderer::reset()
{
	for ( size_t i = 0; i < m_outline_passes.size(); i++ )
	{
		delete m_outline_passes[i];
		delete m_bitmap_passes[i];
	}
	m_outline_passes.clear();
	m_bitmap_passes.clear();
}

GlyphRenderer* GlyphRenderer::init_pass()
{
	reset();
	return this;
}

GlyphRenderer* GlyphRenderer::add_pass(const RenderPassParam& param)
{
	IRenderPass* pass = NULL;

	// add outline pass
	{
		pass = new OutlineRenderPass;
		pass->init(param);
		m_outline_passes.push_back(pass);
	}
	// add bitmap pass
	{
		pass = new BitmapRenderPass;

		RenderPassParam hack_param = param;
		if ( hack_param.stroke_radius > 0 )
		{
			hack_param.stroke_radius = 1 << 6;
		}
		pass->init(hack_param);
		m_bitmap_passes.push_back(pass);
	}

	return this;
}

FT_Error GlyphRenderer::render(FT_Glyph& glyph, GlyphBitmap* glyph_bitmap)
{
	return render(glyph, &glyph_bitmap->bitmap, &glyph_bitmap->top_left_pixels, &glyph_bitmap->advance_pixels);
}

FT_Error GlyphRenderer::render(FT_Glyph& glyph, IBitmap** pbuf, FT_Vector* top_left_pixel, FT_Vector* advance_pixel)
{
	FT_Error error = 0;
	FT_BBox bbox;
	FT_F26Dot6 stroke_radius = 0;
	IBitmap* buf = *pbuf;
	memset(&bbox, 0, sizeof(bbox));
	std::vector<IRenderPass*>* passes = NULL;

	if ( glyph->format == FT_GLYPH_FORMAT_BITMAP )
	{
		passes = &m_bitmap_passes;
	}
	else if ( glyph->format == FT_GLYPH_FORMAT_OUTLINE )
	{
		passes = &m_outline_passes;
	}
	else
	{	
		// do not support!
		return -1;
	}

	for ( size_t i = 0; i < m_outline_passes.size(); i++ )
	{
		stroke_radius = (*passes)[i]->stroke_radius() > stroke_radius ? (*passes)[i]->stroke_radius() : stroke_radius;
		error = (*passes)[i]->pre_render(glyph);
		bbox = intersect_bbox(bbox, (*passes)[i]->cbox());
	}
	align_bbox(bbox);

	if ( buf == NULL )
	{
		buf = new Bitmap_32bits( (bbox.xMax - bbox.xMin) >> 6, (bbox.yMax - bbox.yMin) >> 6 );
		*pbuf = buf;
	}

	for ( size_t i = 0; i < passes->size(); i++ )
	{
		error = (*passes)[i]->post_render(buf, bbox);
	}

	if ( top_left_pixel )
	{
		top_left_pixel->x = bbox.xMin >> 6;
		top_left_pixel->y = (bbox.yMax >> 6) - 1;
	}

	if ( advance_pixel )
	{
		FT_Pos correct = (round_26dot6(stroke_radius * 2) >> 6);
		//advance_pixel->x = round_26dot6(bbox.xMax - bbox.xMin) >> 6;
		//advance_pixel->x = advance_pixel->x < (glyph->advance.x >> 16) ? (glyph->advance.x >> 16) : advance_pixel->x;
		advance_pixel->x = (glyph->advance.x >> 16) + correct;
		advance_pixel->y = round_26dot6(bbox.yMax - bbox.yMin) >> 6;
	}

	return error;
}

FontInfo* FontInfo::create_font(FT_Library library, const char* fontname, FT_UInt width_pt, FT_UInt height_pt, FT_UInt ppi/*=72*/)
{
	return create_font(library, fontname, 0, width_pt, height_pt, ppi);
}
FontInfo* FontInfo::create_font(FT_Library library, const char* fontname, FT_Long face_idx, FT_UInt width_pt, FT_UInt height_pt, FT_UInt ppi)
{
	if ( library )
	{
		FontInfo* f = new FontInfo(library);
		if ( 0 == f->init(library, fontname, face_idx, width_pt, height_pt, ppi) )
		{
			return f;
		}

		delete f;
		return false;
	}

	return NULL;
}


void FontInfo::release()
{
	delete this;
}

FontInfo* FontInfo::add_pass(const RenderPassParam& param)
{
	if ( renderer() == NULL )
	{
		m_private_renderer = new GlyphRenderer;
		m_private_renderer->init_pass();
		set_renderer(m_private_renderer);
	}
	renderer()->add_pass(param);

	int to_extend = 0;
	if ( param.stroke )
	{
		to_extend = (param.stroke_radius + 64) >> 6;
	}

	to_extend = abs(param.translate_x) > to_extend ? abs(param.translate_x) : to_extend;
	to_extend = abs(param.translate_y) > to_extend ? abs(param.translate_y) : to_extend;
	m_extend_pt = m_extend_pt > (FT_UInt)to_extend ? m_extend_pt : (FT_UInt)to_extend;

	return this;
}

FT_UInt FontInfo::render_charcode(FT_ULong char_code, GlyphBitmap* bitmap, FT_UInt prev_idx)
{
	for ( size_t i = 0; i < m_hackfonts.size(); i++ )
	{
		FT_UInt charidx = m_hackfonts[i]->get_char_index(char_code);
		if ( charidx > 0 )
		{
			return m_hackfonts[i]->render_charidx(charidx, bitmap, prev_idx) ? charidx : 0;
		}
	}

	FT_UInt char_idx = get_char_index(char_code);
	if ( char_idx == 0 ) 
		return 0;

	return render_charidx(char_idx, bitmap, prev_idx) ? char_idx : 0;
}

// from char code to char index
FT_UInt FontInfo::get_char_index(FT_ULong charcode)
{
	// if set charset, only chars in charset can be rendered
	if ( m_available_charset != NULL && m_available_charset->find(charcode) == m_available_charset->end() )
	{
		return 0;
	}
	return FT_Get_Char_Index(m_face, charcode);
}

bool FontInfo::render_charidx(FT_UInt char_idx, GlyphBitmap* bitmap, FT_UInt prev_idx)
{
	if ( load_glyph_from_index(char_idx)
		&& 0 == _render_ready_char(bitmap) )
	{
		if ( has_kerning() && prev_idx != 0 )
		{
			FT_Vector kern = get_kerning(prev_idx, char_idx);
			bitmap->kerning_pixels.x = (kern.x >> 6);
			bitmap->kerning_pixels.y = (kern.x >> 6);
		}

		return true;
	}

	return false;
}

// has kerning info
bool FontInfo::has_kerning()
{
	return m_has_kerning;
}
FT_Vector FontInfo::get_kerning(FT_UInt left_idx, FT_UInt right_idx)
{
	FT_Vector delta; 
	delta.x = 0;
	delta.y = 0;
	if ( m_has_kerning ) 
	{ 
		FT_Get_Kerning( m_face, left_idx, right_idx, FT_KERNING_DEFAULT, &delta );  
	} 

	return delta;
}

const char* FontInfo::font_name()
{
	return m_fontname.c_str();
}

FT_Library FontInfo::library()
{
	return m_library;
}

FT_UInt FontInfo::char_width_pt()
{
	return m_char_width;
}

FT_UInt FontInfo::char_height_pt()
{
	return m_char_height;
}

FT_UInt FontInfo::ppi()
{
	return m_ppi;
} 

void FontInfo::set_shift_y(FT_UInt sy)
{
	m_shift_y = sy;
}

FT_UInt FontInfo::extend_pt()
{
	return m_extend_pt;
}

FT_Short FontInfo::underline_position()
{
	return m_underline_position - m_extend_pt;
}

FT_Short FontInfo::underline_thickness()
{
	return m_underline_thickness;
}

FT_Face FontInfo::face()
{
	return m_face;
}

void FontInfo::set_available_charset(std::set<FT_ULong>* charset)
{
	m_available_charset = charset;
}

FontInfo* FontInfo::add_hackfont(const char* fontname, std::set<FT_ULong>* charset, FT_UInt shift_y /*= 0*/)
{
	return add_hackfont(fontname, 0, charset, shift_y);
}

FontInfo* FontInfo::add_hackfont(const char* fontname, FT_Long face_idx, std::set<FT_ULong>* charset, FT_UInt shift_y)
{
	FontInfo* hackfont = FontInfo::create_font(library(), fontname, face_idx, char_width_pt(), char_height_pt(), ppi());
	if ( !hackfont )
	{
		return NULL;
	}

	hackfont->set_available_charset(charset);
	hackfont->set_renderer(renderer());
	hackfont->set_shift_y(shift_y);
	m_hackfonts.push_back(hackfont);
	return hackfont;
}


FT_Error FontInfo::_render_ready_char(GlyphBitmap* bitmap)
{
	FT_Error error;
	FT_Glyph glyph = NULL;
	error = FT_Get_Glyph(current_glyph(), &glyph);
	if ( error != 0 )
		return error;

	error = renderer()->render(glyph, bitmap);
	FT_Done_Glyph(glyph);

	bitmap->top_left_pixels.y += m_shift_y;

	return error;
}

FT_GlyphSlot FontInfo::current_glyph()
{
	return m_face->glyph;
}

bool FontInfo::load_glyph_from_index(FT_UInt char_idx)
{
	return 0 == FT_Load_Glyph(m_face, char_idx, FT_LOAD_DEFAULT);
}

bool FontInfo::load_glyph_from_char(FT_ULong char_code)
{
	FT_UInt char_idx = get_char_index(char_code);

	if ( char_idx == 0 ) 
		return false;

	return load_glyph_from_index(char_idx); 
}

void FontInfo::set_renderer(GlyphRenderer* _renderer)
{
	m_ob_renderer = _renderer;
}

GlyphRenderer* FontInfo::renderer()
{
	return m_ob_renderer;
}

FT_Error FontInfo::init(FT_Library& lib, const char* fontname, FT_Long face_idx, FT_UInt width_pt, FT_UInt height_pt, FT_UInt ppi)
{
	FT_Error error = 0;

	m_fontname = fontname;
	
	error = FT_New_Face(lib, m_fontname.c_str(), face_idx, &m_face);
	if ( error ) return error;

	m_has_kerning = FT_HAS_KERNING(m_face) != 0; 

	// default value
	if (height_pt == 0) height_pt = width_pt;
	if (width_pt == 0) width_pt = height_pt;

	m_ppi = ppi;

	// fixed size
	// choose a nearest one
	if ( m_face->num_fixed_sizes > 0 )
	{
		size_t selected_idx = 0;
		size_t selected_pt_diff = width_pt;
		for ( FT_Int i = 0; i < m_face->num_fixed_sizes; i++ )
		{
			size_t pt_diff = abs( (int)(m_face->available_sizes[i].height - height_pt) );
			if ( pt_diff == 0 )
			{
				selected_idx = i;
				break;
			}
			else if(pt_diff < selected_pt_diff)
			{
				selected_idx = i;
				selected_pt_diff = pt_diff;
			}
		}
		m_char_width = m_face->available_sizes[selected_idx].width;
		m_char_height = m_face->available_sizes[selected_idx].height;
		error = FT_Select_Size(m_face, selected_idx);
		if ( error )
		{
			FT_Done_Face(m_face);
			m_face = NULL;
			return error;
		}
	}
	else
	{
		/* use pixel size */
		error = FT_Set_Pixel_Sizes(m_face, width_pt, height_pt);//FT_Set_Char_Size( m_face, width_pt << 6, height_pt << 6, ppi, ppi );
		if ( error )
		{
			FT_Done_Face(m_face);
			m_face = NULL;
			return error;
		}
		m_char_width = width_pt;
		m_char_height = height_pt;
	}

	/* use default charmap is better! */
	//error = FT_Select_Charmap(m_face , FT_ENCODING_UNICODE);
	//if ( error )
	//{
	//	FT_Done_Face(m_face);
	//	m_face = NULL;
	//}

	m_underline_thickness = height_pt / 20 + 1; //m_face->underline_thickness;
	m_underline_position = 0 - m_underline_thickness; //m_face->underline_position;

	return error;
}

FontInfo::FontInfo(FT_Library lib) 
	: m_library(lib), m_fontname(), m_char_width(0), 
	m_char_height(0), m_ppi(0), m_shift_y(0), m_extend_pt(0),
	m_underline_position(0), m_underline_thickness(0),
	m_face(NULL), m_has_kerning(false),
	m_ob_renderer(NULL), m_private_renderer(NULL),
	m_available_charset(NULL)
{

}

FontInfo::~FontInfo()
{
	for ( size_t i = 0; i < m_hackfonts.size(); i++ )
	{
		delete  m_hackfonts[i];
	}
	m_hackfonts.clear();

	if ( m_private_renderer )
	{
		delete m_private_renderer;
		m_private_renderer = NULL;
	}

	if (m_face)
	{
		FT_Done_Face(m_face);
		m_face = NULL;
	}
}

	
}//namespace dfont

