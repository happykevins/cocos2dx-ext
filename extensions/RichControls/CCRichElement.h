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
#ifndef __CC_RICHELEMENT_H__
#define __CC_RICHELEMENT_H__

#include "CCRichProtocols.h"
#include "CCRichCache.h"

NS_CC_EXT_BEGIN;

//
// Element Base
//	- IRichElement protocol adapter & dispatch skeleton
//
class REleBase : public IRichElement
{
public:
	// attribute key-value pair
	typedef std::map<std::string, std::string> attrs_t;

public:
	// utilities
	static attrs_t*		parseAttributes(const char** attrs);
	static bool			hasAttribute(attrs_t* attrs, const char* attr);

	CCNode* createDrawSolidPolygonNode(RRichCanvas canvas);

public:
	virtual bool parse(class IRichParser* parser, const char** attr = NULL);
	virtual bool composit(class IRichCompositor* compositor);
	virtual void render(RRichCanvas canvas);

	virtual bool pushMetricsState() { return false; }
	virtual bool pushRenderState() { return false; }

	// children
	virtual element_list_t* getChildren();
	virtual void addChildren(IRichElement* child);
	virtual void removeAllChildren();
	// parent
	virtual IRichElement* getParent();
	virtual void setParent(IRichElement* parent);

	virtual int	getID();
	virtual IRichElement* findChildByID(int _id);

	// metrics properties
	virtual RPos getLocalPosition() const { return m_rPos; }
	virtual void setLocalPosition(RPos pos) { m_rPos = pos;}
	virtual void setLocalPositionX(short x) { m_rPos.x = x;}
	virtual void setLocalPositionY(short y) { m_rPos.y = y; }
	virtual RPos getGlobalPosition() { return m_rGlobalPos; }
	
	virtual RMetrics* getMetrics() { return &m_rMetrics; }
	
	virtual RTexture* getTexture() { return &m_rTexture; }
	virtual bool scaleToElementSize() { return false; }
	virtual void setRColor(unsigned int color) { m_rColor = color; }
	virtual unsigned int getColor() { return m_rColor; }
	virtual const char* getFontAlias() { return NULL; }
	virtual bool isBatchedDrawable() { return false; }

	virtual bool canLinewrap() { return true; }
	virtual bool isNewlineBefore() { return false; }
	virtual bool isNewlineFollow() { return false;}
	virtual bool isCachedComposit() { return false;}
	virtual short getBaseline() { return 0; }
	virtual bool needBaselineCorrect() { return false;}

	virtual void onCachedCompositBegin(class ICompositCache* cache, RPos& pen){}
	virtual void onCachedCompositEnd(class ICompositCache* cache, RPos& pen){}

	REleBase();
	virtual ~REleBase();

protected:
	/**
	 * composit events
	 */

	// parse attributes
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs ){ return true; }

	// call just set the local position
	virtual void onCompositStart(class IRichCompositor* compositor) {}
	// call after push the states if claimed
	virtual void onCompositStatePushed(class IRichCompositor* compositor) {}
	// call after visit composit children
	virtual void onCompositChildrenEnd(class IRichCompositor* compositor) {}
	// call after state popped, almost work done, return if this element should be composited 
	virtual bool onCompositFinish(class IRichCompositor* compositor) { return false; }

	/**
	 * render events
	 */

	// call before render children
	virtual void onRenderPrev(RRichCanvas canvas) {}
	// call after render children
	virtual void onRenderPost(RRichCanvas canvas) {}

	int m_rID;

	element_list_t* m_rChildren;
	IRichElement* m_rParent;

	RPos m_rPos;
	RPos m_rGlobalPos;
	RMetrics m_rMetrics;
	RTexture m_rTexture;

	unsigned int m_rColor;
	bool m_rDirty;
};

//////////////////////////////////////////////////////////////////////////
// Common Nodes

//
// SolidPolygon
//	- a common solid-polygon base
//
class REleSolidPolygon : public REleBase
{
public:
	CCNode* createDrawNode(RRichCanvas canvas);
//protected:
//	virtual void onRenderPost(RRichCanvas canvas);
};

//
// Batched Drawable Element
//	- add into atlas of same texture
//
class REleBatchedDrawable : public REleBase
{
public:
	virtual bool isBatchedDrawable() { return true; }
	virtual bool isCachedComposit() { return true; }

	REleBatchedDrawable();

protected:
	virtual bool onCompositFinish(class IRichCompositor* compositor);
	virtual void onRenderPrev(RRichCanvas canvas);
};

//
// Atlas Drawable Helper
//
class RAtlasHelper : public REleBatchedDrawable
{
public:
	virtual bool scaleToElementSize() { return true; }

	virtual void onRenderPrev(RRichCanvas canvas);
	virtual bool isDirty() { return m_rDirty; }
	virtual void setDirty(bool b) { m_rDirty = b; }

	virtual void setGlobalPosition(RPos pos) { m_rGlobalPos = pos; }
};

//
// Glyph Element
//
class REleGlyph : public REleBatchedDrawable
{
public:
	virtual bool canLinewrap() { return true; }
	virtual short getBaseline() { return m_rMetrics.rect.min_y(); }
	virtual const char* getFontAlias() { return m_font_alias.c_str(); }

	REleGlyph(unsigned int charcode);
	virtual ~REleGlyph();

protected:
	virtual void onCompositStart(class IRichCompositor* compositor);

private:
	unsigned int m_charcode;
	struct dfont::GlyphSlot* m_slot;

	std::string m_font_alias;
};

//////////////////////////////////////////////////////////////////////////
// HTML nodes

//
// HTML Node Element
//	- base node for HTML tag
//
class REleHTMLNode : public REleBase
{
public:

	/**
	 * tools for parse HTML attributes
	 */
	static attrs_t*		parseStyle(const std::string& style_str);
	static RMargin		parseMargin(const std::string& str);
	static unsigned int parseColor(const std::string& color_str);
	static ROptSize		parseOptSize(const std::string& str);
	static short		parsePixel(const std::string& str);
	static float		parsePercent(const std::string& str);
	static bool			parseAlignment(const std::string& str, EAlignment& align);
	//static void			processZone(RRect& zone, const ROptSize& width, const ROptSize& height, bool auto_fill_zone=false);
};

//
// HTML P Element
//	- a HTML tag <p>: paragraph
//
//	- support simple css inline-style:
//		- white-space: nowrap
//		- color:#rrggbb[aa]
//		- font:(font alias)
//		- text-align:left|center|right	- horizan alignment
//		- line-height: Npx				- not support yet!
//		- margin: Npx					- used as line spacing
//		- padding: Npx					- used as padding
//
class REleHTMLP : public REleHTMLNode
{
public:
	virtual bool pushMetricsState() { return true; }
	virtual bool pushRenderState() { return true; }
	virtual bool isCachedComposit() { return true; }
	virtual bool isNewlineBefore() { return true; }
	virtual bool isNewlineFollow() { return true; }

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStatePushed(class IRichCompositor* compositor);
	virtual void onCompositChildrenEnd(class IRichCompositor* compositor);
	virtual bool onCompositFinish(class IRichCompositor* compositor);

private:
	RLineCache m_rLineCache;
	std::string m_rFontAlias;
};

//
// Root Element
//
class REleHTMLRoot : public REleHTMLNode
{
public:
	virtual bool pushMetricsState() { return true; }
	virtual bool isCachedComposit() { return true; }
	virtual bool isNewlineBefore() { return true; }
	virtual bool isNewlineFollow() { return true; }

protected:
	virtual bool onCompositFinish(class IRichCompositor* compositor) { return true; }
};

//
// HTML Node NotSupport
//	- not supported tag
//
class REleHTMLNotSupport : public REleHTMLNode
{
};

//
// HTML BR
//	- BR: <br/> - start a new line
//
class REleHTMLBR : public REleHTMLNode
{
public:
	virtual bool isCachedComposit() { return true; }
	virtual bool isNewlineFollow() { return true; }
	virtual bool needBaselineCorrect() { return true; }

protected:
	virtual bool onCompositFinish(class IRichCompositor* compositor);
};

//
// HTML Font
//	- as HTML tag <font>
//
//	- attr: face=<font alias>
//	- attr: color=#rrggbb[aa]
//
class REleHTMLFont : public REleHTMLNode
{
public:
	virtual bool pushRenderState() { return true; }
	REleHTMLFont();

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStatePushed(class IRichCompositor* compositor);

private:
	std::string m_rFont;
	unsigned int m_rColor;
};

//
// HTML Spans
//	- calculate & render spans for characters
//	- supported: underline, background
//
class REleHTMLSpans : public REleHTMLNode
{
public:
	virtual bool pushRenderState() { return true; }

	virtual bool isDrawUnderline() { return m_rDrawUnderline; }
	virtual void setDrawUnderline(bool b) { m_rDrawUnderline = b; }

	virtual bool isDrawBackground() { return m_rDrawBackground; }
	virtual void setDrawBackground(bool b) { m_rDrawBackground = b; }
	virtual unsigned int getBGColor() { return m_rBGColor; }

	REleHTMLSpans();
	virtual ~REleHTMLSpans();

protected:
	virtual void onRenderPost(RRichCanvas canvas);

private:
	void travesalChildrenSpans(
		element_list_t* eles, const char*& font, short& start_span_x, short& span_y, short& underline_thickness, 
		short& end_span_x, short& min_span_y, short& max_span_y, unsigned int& color, bool is_root);
	void clearAllSpans();

protected:
	bool m_rDrawUnderline;
	bool m_rDrawBackground;
	unsigned int m_rBGColor;
	std::vector<REleSolidPolygon*> m_rUnderlineDrawables;
	std::vector<REleSolidPolygon*> m_rBackgroudDrawables;
	std::list<RRect> m_rSpans; // spans
};

//
// HTML U
//	- Underline : <u>
//
class REleHTMLU : public REleHTMLSpans
{
public:
	REleHTMLU();
};


//
// HTML HR
//	- HR: <hr/> - a horizontal line
//
//	- attr: width=w%|w		- the line width
//	- attr: size=n			- the line thickness
//	- css inline-style:
//		- color:#rrggbb[aa]	- line color
//
class REleHTMLHR : public REleBase
{
public:
	virtual bool isNewlineBefore() { return true; }
	virtual bool isNewlineFollow() { return true; }
	virtual bool isCachedComposit() { return true; }

	virtual void onCachedCompositBegin(class ICompositCache* cache, RPos& pen);
	virtual void onCachedCompositEnd(class ICompositCache* cache, RPos& pen);
	REleHTMLHR();

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStart(class IRichCompositor* compositor);
	virtual bool onCompositFinish(class IRichCompositor* compositor);
	virtual void onRenderPost(RRichCanvas canvas);

	short m_rSize;
	ROptSize m_rWidth;
	short m_rTempPadding;
};

//
// HTML Cell
//	- as table tag <td>
//
//	- attr: width=<n>%|<n>
//	- attr: height=<n>%|<n>
//	- attr: align=left|right|center	- content halignment
//	- attr: valign=top|bottom|middle- content valignment
//	- attr: padding=<n>				- text padding
//	- attr: spacing=<n>				- text spacing
//	- attr: nowrap=nowrap			- text do not wrap line
//	- attr: bgcolor=#rrggbb[aa]
//	- attr: bg-image="image file path"
//	- attr: bg-rect="<TOP>px <RIGHT>px <BOTTOM>px <LEFT>px"
//
class REleHTMLCell : public REleHTMLNode
{
	friend class RHTMLTableCache;
public:
	virtual bool pushMetricsState() { return true; }
	virtual void onRenderPrev(RRichCanvas canvas);
	void setIndex(int index) { m_rIndexNumber = index; }
	bool isWidthSet() { return !m_rWidth.isZero(); }

	REleHTMLCell(class REleHTMLRow* row);
	virtual ~REleHTMLCell();

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStatePushed(class IRichCompositor* compositor);
	virtual void onCompositChildrenEnd(class IRichCompositor* compositor);

private:
	class REleHTMLRow* m_rRow;
	RLineCache m_rLineCache;

	bool m_rHAlignSpecified;
	bool m_rVAlignSpecified;
	int m_rIndexNumber;
	EAlignment m_rHAlignment;
	EAlignment m_rVAlignment;
	ROptSize m_rWidth;
	ROptSize m_rHeight;
	RRect m_rContentSize;
	RAtlasHelper m_rBGTexture;
};

//
// HTML Row
//	- as a table tag <tr>
//
//	- attr: align=left|right|center	- content halignment
//	- attr: valign=top|bottom|middle- content valignment
//
class REleHTMLRow : public REleHTMLNode
{
	friend class RHTMLTableCache;
public:
	virtual bool pushMetricsState() { return true; }
	virtual bool isCachedComposit() { return true; }

	virtual std::vector<class REleHTMLCell*>& getCells();
	class REleHTMLTable* getTable();
	short getCellWidth(int index, ROptSize width);

	virtual void addChildren(IRichElement* child);

	REleHTMLRow(class REleHTMLTable* table);

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStatePushed(class IRichCompositor* compositor);
	virtual bool onCompositFinish(class IRichCompositor* compositor) { return true; }

private:
	class REleHTMLTable* m_rTable;
	std::vector<class REleHTMLCell*> m_rCells;

	bool m_rHAlignSpecified;
	bool m_rVAlignSpecified;
	EAlignment m_rHAlignment;
	EAlignment m_rVAlignment;
	short m_rLeftWidth;
};


//
// HTML Table
//	- as a HTML tag <table>
//
//	- attr: width=<n>%|<n>
//	- attr: align=left|right|center	- table halignment relative to other elements
//	- attr: cellspadding=<n>		- cell content padding
//	- attr: cellsspacing=<n>		- cell content spacing
//	- attr: border=<n>				- border thickness
//	- attr: bgcolor=#rrggbb[aa]		- background color
//	- attr: bordercolor=#rrggbb[aa]	- border color
//	- attr: frame=<frame>			- draw outer frame, see EFrame
//	- attr: rules=<rules>			- draw inner frame, see ERules
//
class REleHTMLTable : public REleHTMLNode
{
	friend class RHTMLTableCache;
public:
	// HTML table - frame attribute
	enum EFrame
	{
		e_void,
		e_above,
		e_below,
		e_hsides,
		e_lhs,
		e_rhs,
		e_vsides,
		e_box,
		e_border,
	};
	// HTML table - rules attribute
	enum ERules
	{
		e_none,
		e_groups,
		e_rows,
		e_cols,
		e_all,
	};

	virtual bool pushMetricsState() { return true; }
	virtual bool isCachedComposit() { return true; }
	virtual bool isNewlineBefore() { return true; }
	virtual bool isNewlineFollow()	{ return true; }

	virtual short getZoneWidth() { return m_rZoneWidth; }

	virtual void onCachedCompositBegin(class ICompositCache* cache, RPos& pen);
	virtual void onCachedCompositEnd(class ICompositCache* cache, RPos& pen);

	virtual void addChildren(IRichElement* child);
	REleHTMLTable();

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStatePushed(class IRichCompositor* compositor);
	virtual void onCompositChildrenEnd(class IRichCompositor* compositor);
	virtual bool onCompositFinish(class IRichCompositor* compositor);
	virtual void onRenderPrev(RRichCanvas canvas);
	virtual void drawThicknessLine(short left, short top, short right, short bottom, const ccColor4F& color);
	
private:
	void createTicknessLineNode(RRichCanvas canvas, short left, short top, short right, short bottom, const ccColor4F& color);

	static EFrame parseFrame(const std::string& str);
	static ERules parseRules(const std::string& str);

	RHTMLTableCache m_rTableCache;
	std::vector<class REleHTMLRow*> m_rRows;

	ROptSize m_rWidth;
	short m_rBorder;
	unsigned int m_rBorderColor;
	EFrame m_rFrame;
	ERules m_rRules;

	bool m_rHAlignSpecified;
	EAlignment m_rHAlign;
	EAlignment m_rTempAlign;
	short m_rZoneWidth;
};

//
// HTML Img
//	- static image: <img>
//	- attr: src=<resource-path>
//	- attr: alt=<string>		- not in use yet!
//	- attr: texture-rect="<TOP>px <RIGHT>px <BOTTOM>px <LEFT>px"
//
class REleHTMLImg : public REleBatchedDrawable
{
public:
	virtual bool needBaselineCorrect() { return true; }

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual void onCompositStart(class IRichCompositor* compositor);

private:
	std::string m_filename;
	std::string m_alt;
};

//
// HTML Touchable
//	- Touchable : element can accept touch events
//
class REleHTMLTouchable : public REleHTMLSpans
{
public:
	// touch properties
	virtual void setEnabled(bool b) { m_rEnabled = b;}
	virtual bool isEnabled() { return m_rEnabled; }

	virtual const std::string& getName() const = 0;
	virtual const std::string& getValue() const = 0;

	// check location inside
	virtual bool isLocationInside(CCPoint location);

	// touch events
	virtual bool onTouchBegan(CCNode* container, CCTouch *touch, CCEvent *evt);
	virtual void onTouchMoved(CCNode* container, CCTouch *touch, CCEvent *evt);
	virtual void onTouchEnded(CCNode* container, CCTouch *touch, CCEvent *evt);
	virtual void onTouchCancelled(CCNode* container, CCTouch *touch, CCEvent *evt);

	REleHTMLTouchable();

protected:
	virtual void onCompositStart(class IRichCompositor* compositor);

	bool m_rEnabled;

};


//
// HTML Button
//	- as a HTML tag <button>
//
//	- attr: name=<name string>		- button name
//	- attr: value=<value string>	- button value
//	- attr: bgcolor=#rrggbb[aa]		- back ground color
//
class REleHTMLButton : public REleHTMLTouchable
{
public:
	virtual void setName(const std::string& name) { m_rName = name; }
	virtual const std::string& getName() const { return m_rName; }
	virtual void setValue(const std::string& value) { m_rValue = value; }
	virtual const std::string& getValue() const { return m_rValue; }

protected:
	virtual void onTouchEnded(CCNode* container, CCTouch *touch, CCEvent *evt);
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );

	std::string m_rName;
	std::string m_rValue;
};

//
// HTML Anchor
//	- as a HTML tag <A>
//
//	- attr: name=<name string>		- name
//	- attr: href=<target string>	- target
//	- attr: bgcolor=#rrggbb[aa]		- back ground color
//
class REleHTMLAnchor : public REleHTMLTouchable
{
public:
	virtual void setName(const std::string& name) { m_rName = name; }
	virtual const std::string& getName() const { return m_rName; }
	virtual const std::string& getValue() const { return m_rHref; }
	virtual void setHerf(const std::string& value) { m_rHref = value; }
	virtual const std::string& getHref() const { return m_rHref; }

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );

	std::string m_rName;
	std::string m_rHref;
};

//
// CCB Node
//	- CCB: <ccb> - load "ccbi" file, and add to root overlays layer
//	
//	- attr: src=<file>		- ccbi file name
//	- attr: play="auto"		- auto play after load
//	- attr: anim=<sequence> - auto play sequence name
//
class REleCCBNode : public REleBase
{
public:
	typedef class CCNode* (*ccb_reader_t)(const char* ccbi_file);
	static void registerCCBReader(ccb_reader_t reader);

	virtual bool isCachedComposit() { return true; }
	virtual bool canLinewrap() { return true; }
	virtual bool needBaselineCorrect() { return true;  }

	REleCCBNode();
	virtual ~REleCCBNode();

protected:
	virtual bool onParseAttributes(class IRichParser* parser, attrs_t* attrs );
	virtual bool onCompositFinish(class IRichCompositor* compositor);
	virtual void onRenderPost(RRichCanvas canvas);

private:
	std::string m_filename;
	std::string m_sequence;
	CCNode* m_ccbNode;
	bool m_dirty;
};

NS_CC_EXT_END;

#endif//__CC_RICHELEMENT_H__
