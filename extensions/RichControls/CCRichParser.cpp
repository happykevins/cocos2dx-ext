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
#include "CCRichParser.h"
#include "CCRichElement.h"

#include <stack>

USING_NS_CC;

NS_CC_EXT_BEGIN;

element_list_t* RSimpleHTMLParser::parseFile(const char* filename)
{
	std::string fullpath = CCFileUtils::sharedFileUtils()->fullPathForFilename(filename);
	CCString *contents = CCString::createWithContentsOfFile(fullpath.c_str());

	if ( !contents )
	{
		CCLog("[CCRich] open file failed! %s", filename);
		return NULL;
	}

	return parseString(contents->getCString());
}

element_list_t* RSimpleHTMLParser::parseString(const char* utf8_str)
{
	if ( !utf8_str )
	{
		CCLog("[CCRich] utf8_str is null!");
		return NULL;
	}

	element_list_t* eles = NULL;

	// add a top tag and parse HTML string
	if ( !m_rPlainModeON )
	{
		std::stringstream ss;
		ss << "<root>" << utf8_str << "</root>";
		eles = parseHTMLString(ss.str().c_str());
	}

	// invalid html string!
	if ( !eles )
	{
		eles = new element_list_t;
		m_rCurrentElement = new REleHTMLRoot;
		this->textHandler(this, utf8_str, strlen(utf8_str));

		eles->push_back(m_rCurrentElement);
		m_rCurrentElement = NULL;
	}

	return eles;
}

element_list_t* RSimpleHTMLParser::parseHTMLString(const char* utf8_str)
{
	CCSAXParser parser;
	if ( !parser.init("UTF-8") )
	{
		CCLog("[CCRich] CCSAXParser.init failed!");
		return NULL;
	}

	parser.setDelegator(this);

	element_list_t* elelist = new element_list_t;
	m_rElements = elelist;
	m_rCurrentElement = NULL;

	if ( !parser.parse(utf8_str, strlen(utf8_str)) || elelist->empty() )
	{
		CC_SAFE_DELETE(elelist);
	}

	m_rCurrentElement = NULL;
	m_rElements = NULL;

	return elelist;
}

void RSimpleHTMLParser::startElement(void *ctx, const char *name, const char **atts)
{
	//CCLog("[Parser Start]%s", name);

	IRichElement* element = NULL;

	if ( 0 == strcmp(name, "br") )
	{
		element = new REleHTMLBR;
	}
	else if ( 0 == strcmp(name, "u") )
	{
		element = new REleHTMLU;
	}
	else if ( 0 == strcmp(name, "font") )
	{
		element = new REleHTMLFont;
	}
	else if ( 0 == strcmp(name, "table") )
	{
		element = new REleHTMLTable;
	}
	else if ( 0 == strcmp(name, "tr") )
	{
		if ( dynamic_cast<REleHTMLTable*>(m_rCurrentElement) )
		{
			element = new REleHTMLRow(dynamic_cast<REleHTMLTable*>(m_rCurrentElement));
		}
	}
	else if ( 0 == strcmp(name, "td") )
	{
		if ( dynamic_cast<REleHTMLRow*>(m_rCurrentElement) )
		{
			element = new REleHTMLCell(dynamic_cast<REleHTMLRow*>(m_rCurrentElement));
		}
	}
	else if ( 0 == strcmp(name, "a") )
	{
		element = new REleHTMLAnchor;
	}
	else if ( 0 == strcmp(name, "button") )
	{
		element = new REleHTMLButton;
	}
	else if ( 0 == strcmp(name, "img") )
	{
		element = new REleHTMLImg;
	}
	else if ( 0 == strcmp(name, "ccb") )
	{
		element = new REleCCBNode;
	}
	else if ( 0 == strcmp(name, "hr") )
	{
		element = new REleHTMLHR;
	}
	else if ( 0 == strcmp(name, "p") )
	{
		element = new REleHTMLP;
	}
	else if ( 0 == strcmp(name, "node")
		|| 0 == strcmp(name, "root")
		|| 0 == strcmp(name, "body") )
	{
		element = new REleHTMLRoot;
	}

	if ( !element )
	{
		// tag not supported yet!
		element = new REleHTMLNotSupport;
	}
	
	CCAssert(element, "");

	element->parse(this, atts);

	if ( !m_rCurrentElement )
	{
		m_rElements->push_back(element);
	}
	else
	{
		m_rCurrentElement->addChildren(element);
	}

	m_rCurrentElement = element;
}
void RSimpleHTMLParser::endElement(void *ctx, const char *name)
{
	//CCLog("[Parser End]%s", name);

	CCAssert(m_rCurrentElement, "[CCRich] invalid rich string!");
	m_rCurrentElement = m_rCurrentElement->getParent();
}
void RSimpleHTMLParser::textHandler(void *ctx, const char *s, int len)
{
	//CCLog("[Parser Text]%s", s);

	CCAssert(m_rCurrentElement, "[CCRich]: must specify a parent element!");

	unsigned short* utf16str = cc_utf8_to_utf16(s);
	size_t utf16size = cc_wcslen(utf16str);

	if ( utf16size == 0 )
		return;

	for ( size_t i = 0; i < utf16size; i++ )
	{
		REleGlyph* ele = new REleGlyph(utf16str[i]);
		if ( ele->parse(this) )
		{
			m_rCurrentElement->addChildren(ele);
		}
		else
		{
			CC_SAFE_DELETE(ele);
		}
	}

	CC_SAFE_DELETE_ARRAY(utf16str);
}

RSimpleHTMLParser::RSimpleHTMLParser(IRichNode* container)
: m_rContainer(container)
, m_rPlainModeON(false)
{

}


//////////////////////////////////////////////////////////////////////////

int cc_transfer_oct_value(unsigned short c)
{
	if ( c >= '0' && c <= '9' )
	{
		return c - '0';
	}

	return -1;
}

int cc_transfer_hex_value(unsigned short c)
{
	if ( c >= '0' && c <= '9' )
	{
		return c - '0';
	}
	else if ( c >= 'A' && c <= 'F' )
	{
		return c - 'A' + 10;
	}
	else if ( c >= 'a' && c <= 'f' )
	{
		return c - 'a' + 10;
	}

	return -1;
}

// return consumed char num
int cc_transfer_integer(unsigned short* start, unsigned short* end, int& integer)
{
	int consumed = 0;
	int temp = 0;
	bool minus = false;

	// if reach the end
	if ( start + consumed == end )
	{
		return 0;
	}

	// check if minus
	if ( start[consumed] == '-' )
	{
		minus = true;
		consumed++;

		// if reach the end
		if ( start + consumed == end )
		{
			return 0;
		}
	}

	// hex value
	if ( start + consumed + 2 < end && start[consumed] == '0' && start[consumed+1] == 'x' )
	{
		consumed += 2;
		int hexv = 0;
		int cnt = 0;
		for ( cnt = 0; cnt < 8; cnt++ )
		{
			if ( start + consumed == end )
			{
				break;
			}
			hexv = cc_transfer_hex_value(start[consumed]);
			if ( hexv == -1 )
			{
				break;
			}
			temp = (temp << 4) + hexv;
			consumed++;
		}
		if ( cnt == 0 )
		{
			return 0;
		}

	}
	// oct value
	else
	{
		int octv = 0;
		int cnt = 0;
		for ( cnt = 0; cnt < 10; cnt++ )
		{
			if ( start + consumed == end )
			{
				break;
			}
			octv = cc_transfer_oct_value(start[consumed]);
			if ( octv == -1 )
			{
				break;
			}
			temp = (temp * 10) + octv;
			consumed++;
		}
		if ( cnt == 0 )
		{
			return 0;
		}
	}

	if ( minus )
	{
		temp = -temp;
	}

	integer = temp;

	return consumed;
}

int cc_transfer_angle_brackets_content(unsigned short* start, unsigned short* end, std::string& content)
{
	if ( start >= end || start[0] != '[' )
	{
		return 0;
	}

	int consumed = 1;
	int content_size = 0;

	while ( true )
	{
		// reach the end of string, but back bracket not found!
		if ( start + consumed == end )
		{
			return 0;
		}
		if ( start[consumed] == ']')
		{
			consumed++;
			break;
		}
		consumed++;
		content_size++;
	}

	content.assign(start + 1, start + 1 + content_size);

	return consumed;
}

bool cc_parse_rect(std::string& str, RRect& rect)
{
	int pos = 0;
	// left
	pos = str.find_first_of(',');
	if ( pos == std::string::npos )
	{
		return false;
	}
	std::string temp = str.substr(0, pos);
	int x = atoi(temp.c_str());

	// top
	str = str.substr(pos + 1, str.size());
	pos = str.find_first_of(',');
	if ( pos == std::string::npos )
	{
		return false;
	}
	temp = str.substr(0, pos);
	int y = atoi(temp.c_str());

	// width
	str = str.substr(pos + 1, str.size());
	pos = str.find_first_of(',');
	if ( pos == std::string::npos )
	{
		return false;
	}
	temp = str.substr(0, pos);
	int width = atoi(temp.c_str());

	// height
	str = str.substr(pos + 1, str.size());
	pos = str.find_first_of("|");
	int height = 0;
	if ( pos == std::string::npos )
	{
		height = atoi(str.c_str());
	}
	else
	{
		temp = str.substr(0, pos);
		height = atoi(temp.c_str());
		str = str.substr(pos + 1, str.size());
	}

	rect.pos.x = (short)x;
	rect.pos.y = (short)y;
	rect.size.w = (short)width;
	rect.size.h = (short)height;

	return true;
}

bool cc_parse_image_content(const std::string& content, std::string& filename, RRect& tex_rect, RRect& composit_rect)
{
	std::string left = content;

	size_t pos = left.find_first_of('|');

	// no rect info.
	if ( pos == std::string::npos )
	{
		filename = content;
		return filename.size() > 0;
	}

	filename = left.substr(0, pos);
	left = left.substr(pos + 1, left.size());

	if ( !cc_parse_rect(left, tex_rect) )
	{
		return false;
	}

	if ( !cc_parse_rect(left, composit_rect) )
	{
		return true;
	}

	return true;
}

/**
int cc_transfer_phrase(unsigned short* start, unsigned short* end, IRichElement*& element)
{
	int consumed = 0;
	if ( start[consumed] != cc_rich_char_slash )
	{
		return consumed;
	}

	consumed++;

	// reach the end
	if ( start + consumed == end )
	{
		return 0;
	}

	// a slash char
	if ( start[consumed] == cc_rich_char_slash )
	{
		consumed++;
		element = new REleGlyph(cc_rich_char_slash);
	}
	// a new-line char
	else if ( start[consumed] == 'n' )
	{
		consumed++;
		element = new REleCAEnter();
	}
	// a tab control
	else if ( start[consumed] == 't' )
	{
		consumed++;

		int integer = 0;
		int cnt = cc_transfer_integer(start + consumed, end, integer);

		element = new REleCATab( integer );
		consumed += cnt;
	}
	// push font
	else if ( start[consumed] == 'f' )
	{
		consumed++;
		std::string content;
		int cnt = cc_transfer_angle_brackets_content(start + consumed, end, content);

		if ( cnt > 0 )
		{
			element = new RElePASetFont(content);
			consumed += cnt;
		}
		else
			return 0;
	}
	// underline: '\u+' - underline on; '\u-' - underline off
	else if ( start[consumed] == 'u' )
	{
		consumed++;

		if ( start[consumed] == '+' )
		{
			consumed++;
			element = new REleLAUnderline(true);
		}
		else if ( start[consumed] == '-' )
		{
			consumed++;
			element = new REleLAUnderline(false);
		}
		else
		{
			return 0;
		}
	}
	// change color
	else if ( start[consumed] == 'c' )
	{
		consumed++;

		int integer = 0;
		int cnt = cc_transfer_integer(start + consumed, end, integer);

		if ( cnt > 0 )
		{
			consumed += cnt;
			element = new RElePASetColor( (unsigned int)integer );
		}
		else
		{
			return 0;
		}
	}
	// image file name
	else if ( start[consumed] == 'i' )
	{
		consumed++;
		std::string content;
		int cnt = cc_transfer_angle_brackets_content(start + consumed, end, content);

		if ( cnt > 0 )
		{
			std::string filename;
			RRect tex_rect;
			RRect composit_rect;
			if ( cc_parse_image_content(content, filename, tex_rect, composit_rect) )
			{
				consumed += cnt;
				element = new REleStaticImage( filename.c_str(), tex_rect, composit_rect);
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	// if it is a integer
	else
	{
		int integer = 0;
		int cnt = cc_transfer_integer(start + consumed, end, integer);

		if ( cnt > 0 )
		{
			consumed += cnt;
			element = new REleGlyph( (unsigned short)integer );
		}
		else
		{
			return consumed;
		}
	}

	return consumed;
}
*/

//////////////////////////////////////////////////////////////////////////

NS_CC_EXT_END;

