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

#ifndef CCELL_H_
#define CCELL_H_

#include <cstddef>
#include <string>
#include <map>
#include <list>
#include "cells.h"

namespace cells
{

class CCDF;

typedef std::list<class CCell*> celllist_t;


/*
 * CCell
 *	cells的最小管理单元
 *	1.单元名字
 *	2.单元hash
 *	3.单元类型
 *	4.单元状态
 */
class CCell
{
public:
	enum ecellstate_t
	{
		unknow, loading, verified, error
	};

	CCell(const std::string& _name, const std::string& _hash, estatetype_t _celltype = e_state_file_common);
	~CCell();

	const std::string m_name;
	std::string m_hash;
	std::string m_zhash;
	props_t m_props;
	volatile int m_cellstate;
	estatetype_t m_celltype;

	size_t m_download_times;
	eloaderror_t m_errorno;
	eziptype_t	m_ziptype;

	CCDF* m_cdf;
	CProgressWatcher* m_watcher;
};

/*
 * CCDF - Cell Description File
 * 	1. 记载描述表内部元素
 * 	2. 可以嵌套CDF类型的cell
 */
class CCDF
{
public:
	CCDF(const CCell* _cell);
	~CCDF();

	bool serialize();
	bool deserialize();

	const CCell* m_hostcell;
	celllist_t   m_subcells;
	props_t	m_props;
};

} /* namespace cells */
#endif /* CCELL_H_ */
