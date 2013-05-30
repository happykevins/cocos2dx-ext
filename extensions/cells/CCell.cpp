/****************************************************************************
 Copyright (c) 2012-2013 Kevin Sun and RenRen Games

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

#include "CCell.h"

#include <sstream>

namespace cells
{

CCell::CCell(const std::string& _name, const std::string& _hash /*= NULL*/,
		estatetype_t _celltype /*= common*/) :
		m_name(_name), m_hash(_hash), m_cellstate(unknow), m_celltype(_celltype), 
		m_download_times(0), m_errorno(e_loaderr_ok), m_ziptype(e_zip_none), m_cdf(NULL), 
		m_watcher(NULL)
{
}

CCell::~CCell()
{
	if (m_cdf)
	{
		delete m_cdf;
		m_cdf = NULL;
	}
}

CCDF::CCDF(const CCell* _cell) :
		m_hostcell(_cell)
{
}
CCDF::~CCDF()
{
}

} /* namespace cells */
