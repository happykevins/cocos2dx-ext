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

#ifndef CUTILS_H_
#define CUTILS_H_

#include "CPlatform.h"
#include "md5.h"
#include "zpip.h"

namespace cells
{

class CUtils
{
public:
#if defined(_WIN32)
	inline static void sleep(unsigned int millisec)
	{
		Sleep(millisec);
	}
	inline static void yield()
	{
		Sleep(0);
	}
#else // __linux__ 
	inline static void sleep(unsigned int millisec)
	{
		usleep(millisec * 1000);
	}
	inline static void yield()
	{
		//pthread_yield();
		usleep(0);
	}
#endif

	inline static int atoi(const char* str)
	{
		//return 0;
		return str == NULL ? 0 : ::atoi(str);
	}

	// get time of day
	static bool gettimeofday(timeval_t* tv, void* tz);

	// get elapsed seconds quickly
	static double gettime_seconds();

	// zlib compress & decompress
	static int compress(const char* file_in, const char* file_out, int level = -1);
	static int compress_fd(FILE* fin, FILE* fout, int level = -1);
	static int decompress(const char* file_in, const char* file_out, double* pnow = NULL, double* ptotal = NULL);
	static bool decompress_pkg(const char* filename, const char* outpath, double* pnow = NULL, double* ptotal = NULL);
	//static int decompress_fd(FILE* fin, FILE* fout);

	// hash utils
	static std::string filehash_md5str(FILE* fp, char* buf, size_t buf_size, double* pnow = NULL, double* ptotal = NULL);

	// directory access
	static bool access(const char* path, int mode);

	// make directory
	static bool mkdir(const char* path);

	// build path directorys
	static bool builddir(const char* path);

	// rename file
	static bool rename(const char* from, const char* to);

	// remove file
	static bool remove(const char* path);

	// trim
	static std::string str_trim(std::string s);

	// replace char
	static size_t str_replace_ch(std::string& str, char which, char to);


};

} /* namespace cells */
#endif /* CUTILS_H_ */
