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
#ifndef __DFONT_UTILITY_H__ 
#define __DFONT_UTILITY_H__

#include "dfont_config.h"

#include <set>

namespace dfont
{
	// get system font directory
	extern const char* get_systemfont_path();

	// get system default ppi
	extern int get_system_default_ppi();

	// get system default font size
	extern int get_prefered_default_fontsize();

	// get system_default font
	extern const char* get_system_default_fontfile();

	// get system default latin font
	extern const char* get_system_default_hacklatin_fontfile();

	// default initializer for dfont
	extern void dfont_default_initialize();

	// latin charactor set
	extern std::set<unsigned long>* latin_charset();

	// dump to tga file
#if _DFONT_DEBUG
	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned int uint32;

	bool dump2tga(const std::string &filename, const unsigned int *pxl, uint16 width, uint16 height);
#endif//_DFONT_DEBUG
}

#endif//__DFONT_UTILITY_H__
