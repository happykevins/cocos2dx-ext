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
#include "dfont_utility.h"
#include "dfont_render.h"
#include "dfont_manager.h"

#include <cocos2d.h>
#include <fstream>

#if defined(_MSC_VER)

#define NO_WIN32_LEAN_AND_MEAN
#include <windows.h>

#elif defined(__GNUC__)

#else
#error  "dfont do not support this os!"
#endif

using namespace cocos2d;

namespace dfont 
{

const char* dfont_default_fontpath = NULL;
const char* dfont_default_fontfile = NULL;
int			dfont_default_ppi = 96;
int			dfont_default_size = 18;
unsigned int dfont_default_color = 0xffffffff;

//
// platform utilities
//
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

char default_fontpath_buf[256] = {0};
const char* get_systemfont_path()
{
	GetWindowsDirectoryA(default_fontpath_buf, 256);
	sprintf_s(default_fontpath_buf, 256, "%s/%s", default_fontpath_buf, "/fonts");
	return default_fontpath_buf;
}

int get_system_default_ppi()
{
	return 0;
}

int get_prefered_default_fontsize()
{
	return 18;
}

const char* get_system_default_fontfile()
{
	return "simsun.ttc";
}

const char* get_system_fallback_fontfile()
{
	return "arial.ttf";
}

const char* get_system_default_hacklatin_fontfile()
{
	return "courf.fon";
}

int get_system_default_hacklatin_fontshifty()
{
	return 1;
}

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

const char* get_systemfont_path()
{
	return "/System/Library/Fonts/Cache";
}

int get_system_default_ppi()
{
	return 0;
}

int get_prefered_default_fontsize()
{
	return 18;
}

const char* get_system_default_fontfile()
{
	return "STHeiti Light.ttc";
}

const char* get_system_fallback_fontfile()
{
	return "_H_Helvetica.ttf";
}

const char* get_system_default_hacklatin_fontfile()
{
	return NULL;
}

int get_system_default_hacklatin_fontshifty()
{
	return 0;
}

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

const char* get_systemfont_path()
{
	return "/System/Library/Fonts";
}

int get_system_default_ppi()
{
	return 0;
}

int get_prefered_default_fontsize()
{
	return 18;
}

const char* get_system_default_fontfile()
{
	return "STHeiti Light.ttc";
}

const char* get_system_fallback_fontfile()
{
	return "Helvetica.dfont";
}

const char* get_system_default_hacklatin_fontfile()
{
	return NULL;
}

int get_system_default_hacklatin_fontshifty()
{
	return 0;
}

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
const char* get_systemfont_path()
{
	return "/system/fonts";
}

int get_system_default_ppi()
{
	return 0;
}

int get_prefered_default_fontsize()
{
	return 18;
}

const char* get_system_default_fontfile()
{
	return "DroidSansFallback.ttf";
}

const char* get_system_fallback_fontfile()
{
	return "DroidSansFallback.ttf";
}

const char* get_system_default_hacklatin_fontfile()
{
	return NULL;
}

int get_system_default_hacklatin_fontshifty()
{
	return 0;
}

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
const char* get_systemfont_path()
{
	return "/usr/share/fonts";
}

int get_system_default_ppi()
{
	return 0;
}

int get_prefered_default_fontsize()
{
	return 18;
}

const char* get_system_default_fontfile()
{
	return "truetype/wqy/wqy-microhei.ttc";
}

const char* get_system_fallback_fontfile()
{
	return "truetype/freefont/FreeSerif.ttf";
}

const char* get_system_default_hacklatin_fontfile()
{
	return NULL;
}

int get_system_default_hacklatin_fontshifty()
{
	return 0;
}


#else
#error  "dfont do not support this os!"
#endif

//////////////////////////////////////////////////////////////////////////
//
// dfont system default initializer
//
void dfont_default_initialize()
{
	dfont_default_fontpath	= get_systemfont_path();
	dfont_default_ppi		= get_system_default_ppi();
	dfont_default_fontfile	= get_system_default_fontfile();
	dfont_default_size		= get_prefered_default_fontsize();

	CCAssert(dfont_default_fontpath, "");
	CCAssert(dfont_default_fontfile, "");
	CCFileUtils::sharedFileUtils()->addSearchPath(dfont_default_fontpath);

	// add default font
	FontCatalog* font_catalog = FontFactory::instance()->create_font(
		DFONT_DEFAULT_FONTALIAS, 
		dfont_default_fontfile, 
		dfont_default_color, 
		dfont_default_size, 
		e_plain);

	// fallback
	if ( !font_catalog )
	{
		dfont_default_fontfile	= get_system_fallback_fontfile();
		font_catalog = FontFactory::instance()->create_font(
			DFONT_DEFAULT_FONTALIAS, 
			dfont_default_fontfile, 
			dfont_default_color, 
			dfont_default_size, 
			e_plain);
	}
	else
	{
		// hack latin charset?
		if ( get_system_default_hacklatin_fontfile() )
		{
			std::string fullpath = CCFileUtils::sharedFileUtils()
				->fullPathForFilename(get_system_default_hacklatin_fontfile());

			/*FontInfo* hackfont =*/ font_catalog->add_hackfont(
				fullpath.c_str(), latin_charset(), 
				get_system_default_hacklatin_fontshifty());
		}
	}
}

std::set<unsigned long>* latin_charset()
{
	static std::set<unsigned long> latinset;

	for ( unsigned long i = 0; i < 256; i++ )
	{
		latinset.insert(i);
	}

	return &latinset;
}


//////////////////////////////////////////////////////////////////////////
// write to TGA file, for dump textures
#if _DFONT_DEBUG

	// TGA Header struct to make it simple to dump a TGA to disc.
#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#pragma pack(1)               // Dont pad the following struct.
#endif

	struct TGAHeader
	{
		uint8   idLength,           // Length of optional identification sequence.
			paletteType,        // Is a palette present? (1=yes)
			imageType;          // Image data type (0=none, 1=indexed, 2=rgb,
		// 3=grey, +8=rle packed).
		uint16  firstPaletteEntry,  // First palette index, if present.
			numPaletteEntries;  // Number of palette entries, if present.
		uint8   paletteBits;        // Number of bits per palette entry.
		uint16  x,                  // Horiz. pixel coord. of lower left of image.
			y,                  // Vert. pixel coord. of lower left of image.
			width,              // Image width in pixels.
			height;             // Image height in pixels.
		uint8   depth,              // Image color depth (bits per pixel).
			descriptor;         // Image attribute flags.
	};

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif


	bool
		dump2tga(const std::string &filename,
		const unsigned int *pxl,
		uint16 width,
		uint16 height)
	{
		std::ofstream file(filename.c_str(), std::ios::binary);
		if (file)
		{
			TGAHeader header;
			memset(&header, 0, sizeof(TGAHeader));
			header.imageType  = 2;
			header.width = width;
			header.height = height;
			header.depth = 32;
			header.descriptor = 0x20;

			file.write((const char *)&header, sizeof(TGAHeader));
			file.write((const char *)pxl, sizeof(unsigned int) * width * height);

			return true;
		}
		return false;
	}

#endif// _DFONT_DEBUG
//////////////////////////////////////////////////////////////////////////

} // namespace dfont
