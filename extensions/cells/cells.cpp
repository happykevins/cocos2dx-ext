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

#include "cells.h"
#include "CCells.h"


namespace cells
{

const char* CDF_VERSION = "version";
const char* CDF_LOADALL	= "loadall";
const char* CDF_CELL_CDF = "cdf";
const char* CDF_CELL_NAME = "name";
const char* CDF_CELL_LOAD = "load";
const char* CDF_CELL_HASH = "hash";
const char* CDF_CELL_SIZE = "size";
const char* CDF_CELL_ZHASH = "zhash";
const char* CDF_CELL_ZSIZE = "zsize";
const char* CDF_CELL_ZIP = "zip";
const char* CDF_TAG_PKG = "pkg";
const char* CDF_TAG_CELL = "cell";

// default value
CRegulation::CRegulation() : 
	worker_thread_num(CELLS_DEFAULT_WORKERNUM), max_download_speed(CELLS_DOWNLOAD_SPEED_NOLIMIT),
	auto_dispatch(true), only_local_mode(false), 
	enable_ghost_mode(false), max_ghost_download_speed(CELLS_GHOST_DOWNLOAD_SPEED),
	enable_free_download(false), 
	//zip_type(e_zip_none), zip_cdf(false),
	remote_zipfile_suffix(CELLS_REMOTE_ZIPFILE_SUFFIX),
	tempfile_suffix(CELLS_DEFAULT_TEMP_SUFFIX), temphash_suffix(CELLS_DEFAULT_HASH_SUFFIX)
{
}

float CProgressWatcher::progress()
{
	switch ( step )
	{
	case e_initial:
	case e_error:
		return .0f;
	case e_finish:
		return 100.0f;
	case e_verify_local:
	case e_verify_download:
	case e_unzip:
	case e_download:
		break;
	}

	if ( total > 0 )
	{
		return int((now / total) * 10000) * 0.01f;
	}

	return .0f;
}

void CProgressWatcher::set_step(estep_t _step) 
{ 
	step = _step; 
	now = .0f; 
	total = .0f; 
}

CProgressWatcher::CProgressWatcher() 
	: step(e_initial), now(.0f), total(.0f)
{
}

CellsHandler* cells_create(const CRegulation& rule)
{
	CCells* handler = new CCells;
	if ( handler->initialize(rule) )
	{
		return handler;
	}
	delete handler;
	return NULL;
}

void cells_destroy(CellsHandler* handler)
{
	if ( !handler )
		return;

	CCells* impl = dynamic_cast<CCells*>(handler);
	if ( impl )
	{
		impl->destroy();
	}

	delete handler;
}

}//namespace cells