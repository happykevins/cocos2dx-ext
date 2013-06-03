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
#ifndef __CC_RICHCACHE_H__
#define __CC_RICHCACHE_H__

#include "CCRichProtocols.h"

NS_CC_EXT_BEGIN;

class RCacheBase : public ICompositCache
{
public:
	// alignments properties
	virtual EAlignment getHAlign() { return m_rHAlign; }
	virtual EAlignment getVAlign() { return m_rVAlign; }
	virtual void setHAlign(EAlignment align) { m_rHAlign = align; }
	virtual void setVAlign(EAlignment align) { m_rVAlign = align; }
	
	// line height
	virtual short getLineHeight() { return m_rLineHeight; }
	virtual void setLineHeight(short v) { m_rLineHeight = v; }

	// margin&padding properties
	virtual void setSpacing(short v) { m_rSpacing = v; }
	virtual void setPadding(short v) { m_rPadding = v; }
	virtual short getSpacing() { return m_rSpacing; }
	virtual short getPadding() { return m_rPadding; }

	// line wrap
	virtual void setWrapline(bool wrap) { m_rWrapLine = wrap; }
	virtual bool isWrapline() { return m_rWrapLine; }

	RCacheBase();

protected:
	EAlignment m_rHAlign;
	EAlignment m_rVAlign;
	short m_rLineHeight;
	short m_rSpacing;
	short m_rPadding;
	bool m_rWrapLine;
};

//
// a line-cached compositor
//
class RLineCache : public RCacheBase
{
public:
	virtual void appendElement(IRichElement* ele);
	virtual element_list_t* getCachedElements();
	virtual RRect flush(class IRichCompositor* compositor);
	virtual void clear();

	RLineCache();

protected:
	element_list_t m_rCachedLine;
	
	short m_rBaselinePos;
};


//
// a table row cached compositor
//
class RHTMLTableCache : public RCacheBase
{
public:
	virtual void appendElement(IRichElement* ele);
	virtual element_list_t* getCachedElements();
	virtual void clear();
	virtual RRect flush(class IRichCompositor* compositor);
	void setTable(class REleHTMLTable* table);

	RHTMLTableCache();

private:
	void recompositCell(class REleHTMLCell* cell);
	void travesalRecompositChildren(element_list_t* eles, short x_fixed, short y_fixed);

protected:
	element_list_t m_rCached;
	class REleHTMLTable* m_rTable;
};

NS_CC_EXT_END;

#endif//__CC_RICHCACHE_H__
