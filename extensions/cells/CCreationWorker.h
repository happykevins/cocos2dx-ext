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

#ifndef CCREATIONWORKER_H_
#define CCREATIONWORKER_H_

#include "CContainer.h"
#include "CCell.h"
#include "CDownloader.h"

namespace cells
{

class CCreationFactory;

#define CWORKER_BUFFER_SIZE 16384

/*
 * CCreationWorker
 * 	创造cell工作线程
 * 	1.验证本地cell是否合法
 * 	2.在指定的url下载cell
 */
class CCreationWorker
{
public:
	CCreationWorker(CCreationFactory* host, size_t no);
	virtual ~CCreationWorker();

public:
	virtual void post_work(CCell* cell);
	virtual void do_work();
	virtual size_t workload();		// 负载情况
	size_t get_downloadbytes();
	virtual const char* get_local_path();

protected:
	virtual bool work_verify_local(CCell* cell, FILE* fp);
	virtual eloaderror_t work_download_remote(CCell* cell, bool zip_mark, const char* localurl, const char* localhashurl);
	virtual bool work_decompress(const char* tmplocalurl, const char* localurl, struct CProgressWatcher* watcher, bool pkg=false);
	virtual bool work_patchup_cell(CCell* cell, const char* localurl);
	virtual void work_finished(CCell* cell);

	virtual size_t calc_maxspeed(); 

private:
	static void* working(void* context);

	// downloader callback
	// @return - should flush to disk
	bool on_download_bytes(size_t bytes);

protected:
	CCreationFactory* m_host;
	const size_t m_workno;

private:
	volatile bool m_working;
	pthread_t m_thread;
	sem_t* m_psem;
	sem_t m_sem;
	CQueue<CCell*> m_queue;
	CDownloader m_downloadhandle;
	char m_databuf[CWORKER_BUFFER_SIZE];

	// congestion stat.
	volatile size_t m_downloadbytes;
	size_t m_cachedbytes;			// no flush bytes

	friend class CDownloader;
	friend class CDFParser;
};

/*
 * CGhostWorker - ghost模式的工作线程
 */
class CGhostWorker : public CCreationWorker
{
public:
	CGhostWorker(CCreationFactory* host, size_t no);
	virtual ~CGhostWorker();

protected:
	virtual size_t calc_maxspeed(); 
};

} /* namespace cells */
#endif /* CCREATIONWORKER_H_ */
