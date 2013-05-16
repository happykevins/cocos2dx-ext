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
#ifndef __CC_RICHPARSER_H__
#define __CC_RICHPARSER_H__

#include "CCRichProtocols.h"

NS_CC_EXT_BEGIN;

class RSimpleHTMLParser : public IRichParser, public CCSAXDelegator
{
public:
	// from IRichParser protocol
	virtual element_list_t* parseString(const char* utf8_str);
	virtual element_list_t* parseFile(const char* filename);
	virtual class IRichNode* getContainer() { return m_rContainer; }

	virtual bool isPlainMode() { return m_rPlainModeON; }
	virtual void setPlainMode(bool on) { m_rPlainModeON = on; }

	RSimpleHTMLParser(IRichNode* container);

protected:
	virtual void startElement(void *ctx, const char *name, const char **atts);
	virtual void endElement(void *ctx, const char *name);
	virtual void textHandler(void *ctx, const char *s, int len);

private:
	element_list_t* parseHTMLString(const char* utf8_str);

	IRichNode* m_rContainer;
	element_list_t* m_rElements;
	IRichElement* m_rCurrentElement;
	bool m_rPlainModeON;
};

NS_CC_EXT_END;

#endif//__CC_RICHPARSER_H__
