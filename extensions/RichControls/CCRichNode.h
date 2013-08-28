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
#ifndef __CC_RICHNODE_H__
#define __CC_RICHNODE_H__

#include "CCRichProtocols.h"

NS_CC_EXT_BEGIN;

//
// a base node for rich controls
//
class CCRichNode: public CCNode, public IRichNode
{
public:
	// map: texture - rich atlas
	typedef std::map<CCTexture2D*, class CCRichAtlas*> atlas_map_t;
	// map: color - atlas_map_t
	typedef std::map<unsigned int, atlas_map_t*> color_map_t;

public:
	//
	// implements IRichNode protocol
	//
	virtual IRichParser* getParser();
	virtual IRichCompositor* getCompositor();
	virtual RSize getActualSize();

	virtual RSize getPreferredSize();
	virtual void setPreferredSize(RSize size);

	virtual void setStringUTF8(const char* utf8_str);
	virtual void appendStringUTF8(const char* utf8_str);
	virtual const char* getStringUTF8();
	virtual IRichAtlas* findAtlas(class CCTexture2D* texture, unsigned int color_rgba, int zorder = ZORDER_CONTEXT);
	virtual void addOverlay(IRichElement* overlay);
	virtual void addCCNode(class CCNode* node);
	virtual void removeCCNode(class CCNode* node);
	class CCRichOverlay* getOverlay();

	// 
	// CCNode functions
	// 
	virtual void draw();

	//
	// Utilities
	//
	virtual void setPlainMode(bool on);
	virtual bool isPlainMode() { return getParser()->isPlainMode(); }

	virtual void setDefaultFontAlias(const char* font_alias);
	virtual const char* getDefaultFontAlias();
	virtual void setDefaultColor(unsigned int color);
	virtual unsigned int getDefaultColor();
	virtual void setDefaultAlignment(EAlignment align);
	virtual EAlignment getDefaultAlignment();
	virtual bool isDefaultWrapline();
	virtual void setDefaultWrapline(bool wrapline);
	virtual short getDefaultSpacing();
	virtual void setDefaultSpacing(short spacing);
	virtual short getDefaultPadding();
	virtual void setDefaultPadding(short padding);

	virtual bool initialize() = 0;

	CCRichNode(class CCNode* container);
	virtual ~CCRichNode();

private:
	void processRichString(const char* utf8_str);
	void updateAll();
	void updateContentSize();
	void clearStates();
	void clearRichElements();
	void clearAtlasMap();
	class CCRichAtlas* findColoredTextureAtlas(CCTexture2D* texture, unsigned int color_rgba, int zorder);

protected:
	class CCNode* m_rContainer;

	IRichParser* m_rParser;
	IRichCompositor* m_rCompositor;

	std::string m_rRichString;
	element_list_t m_rElements;

	RSize m_rPreferedSize;

	color_map_t m_rAtlasMap;
	std::vector<class CCRichAtlas*> m_rAtlasList;
	class CCRichOverlay* m_rOverlays;
};

//
// HTML Node
//
class CCHTMLNode : public CCRichNode
{
public:
	static CCHTMLNode* createWithContainer(class CCNode* container);

	bool initialize();

private:
	CCHTMLNode(class CCNode* container);
};

NS_CC_EXT_END;


#endif//__CC_RICHNODE_H__

