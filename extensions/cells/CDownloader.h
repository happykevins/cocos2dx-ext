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

#ifndef CDOWNLOADER_H_
#define CDOWNLOADER_H_

#include <cstddef>
#include <stdio.h>

namespace cells
{

class CCell;
class CCreationWorker;
struct CProgressWatcher;

typedef void download_handle_t;

class CDownloader
{
public:
	enum edownloaderr_t
	{
		e_downloaderr_ok = 0,
		e_downloaderr_connect,
		e_downloaderr_timeout,
		e_downloaderr_notfound,
		e_downloaderr_other_nobp,
	};
public:
	CDownloader(CCreationWorker* host);
	~CDownloader();

	edownloaderr_t download(const char* url, FILE* fp, bool bp_resume, size_t bp_range_begin, CProgressWatcher* watcher = NULL);

private:
	static size_t process_data(void* buffer, size_t size, size_t nmemb, void* context);
	static int progress(void *ctx, double dlTotal, double dlNow, double upTotal, double upNow);

	CCreationWorker* m_host;
	download_handle_t* m_handle;
	FILE* m_stream;
};

} /* namespace cells */
#endif /* CDOWNLOADER_H_ */
