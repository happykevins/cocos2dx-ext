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

#include "CUtils.h"

#include <string>

#if USING_COCOS2DX
#include <platform/CCFileUtils.h>
#include <support/zip_support/unzip.h>
using namespace cocos2d;
#else
#	include <unzip.h>
#endif

#define ZIP_BUFFER_SIZE 8192
#define MAX_FILENAME   512

namespace cells
{

bool CUtils::gettimeofday(timeval_t* tv, void* tz)
{
#if defined(_WIN32)
	if (tv)
	{
		LARGE_INTEGER liTime, liFreq;
		QueryPerformanceFrequency( &liFreq );
		QueryPerformanceCounter( &liTime );
		tv->tv_sec     = (long)( liTime.QuadPart / liFreq.QuadPart );
		tv->tv_usec    = (long)( liTime.QuadPart * 1000000.0 / liFreq.QuadPart - tv->tv_sec * 1000000.0 );
		return true;
	}
	return false;
#else
	return ::gettimeofday((timeval*)tv, NULL/*(__timezone_ptr_t)tz*/) == 0;
#endif
}

double CUtils::gettime_seconds()
{
	timeval_t tv;
	if ( CUtils::gettimeofday(&tv, NULL) )
	{
		return tv.tv_sec + tv.tv_usec / 1000000.0f;
	}

	return .0f;
}

int CUtils::compress(const char* file_in, const char* file_out, int level /*= -1*/)
{
	FILE* fin = fopen(file_in, "rb");
	if ( !fin )
	{
		CLogD("CUtils::compress: open input file failed %s\n", file_in);
		return -1;
	}
	FILE* fout = fopen(file_out, "wb+");
	if ( !fout )
	{
		CLogD("CUtils::compress: open output file failed %s\n", file_out);
		fclose(fin);
		return -1;
	}

	int ret = def(fin, fout, level);

	fclose(fout);
	fclose(fin);
	return ret;
}

int CUtils::compress_fd(FILE* fin, FILE* fout, int level)
{
	return def(fin, fout, level);
}

int CUtils::decompress(const char* file_in, const char* file_out, double* pnow /*= NULL*/, double* ptotal /*= NULL*/)
{
	FILE* fin = fopen(file_in, "rb");
	if ( !fin )
	{
		CLogD("CUtils::decompress: open input file failed %s\n", file_in);
		return -1;
	}

	if ( ptotal )
	{
		fseek(fin, 0, SEEK_END); 
		*ptotal = ftell(fin);
		fseek(fin, 0, SEEK_SET);
	}

	FILE* fout = fopen(file_out, "wb+");
	if ( !fout )
	{
		CLogD("CUtils::decompress: open output file failed %s\n", file_out);
		fclose(fin);
		return -1;
	}

	int ret = inf(fin, fout, pnow);

	fclose(fout);
	fclose(fin);
	return ret;
}

bool CUtils::decompress_pkg(const char* filename, const char* outpath, double* pnow /*= NULL*/, double* ptotal /*= NULL*/)
{
	// Open the zip file
	unzFile zipfile = unzOpen(filename);

	if (! zipfile)
	{
		CLogE("[Cells] can not open zip file %s", filename);
		return false;
	}

	// Get info about the zip file
	unz_global_info global_info;
	if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
	{
		CLogE("[Cells] can not read file global info of %s", filename);
		unzClose(zipfile);
	}

	// progress total
	if ( ptotal )
	{
		*ptotal = global_info.number_entry;
	}

	// Buffer to hold data read from the zip file
	char readBuffer[ZIP_BUFFER_SIZE];

	CLogD("start uncompressing");

	// Loop to extract all files.
	uLong i;
	for (i = 0; i < global_info.number_entry; ++i)
	{
		// Get info about current file.
		unz_file_info fileInfo;
		char fileName[MAX_FILENAME];
		if (unzGetCurrentFileInfo(zipfile,
			&fileInfo,
			fileName,
			MAX_FILENAME,
			NULL,
			0,
			NULL,
			0) != UNZ_OK)
		{
			CLogE("can not read file info");
			unzClose(zipfile);
			return false;
		}

		std::string fullPath = outpath;
		fullPath += fileName;

		// Check if this entry is a directory or a file.
		const size_t filenameLength = strlen(fileName);
		if (fileName[filenameLength-1] == '/')
		{
			// Entry is a direcotry, so create it.
			// If the directory exists, it will failed scilently.
			if (!builddir(fullPath.c_str()))
			{
				CLogE("can not create directory %s", fullPath.c_str());
				unzClose(zipfile);
				return false;
			}
		}
		else
		{
			// Entry is a file, so extract it.

			// Open current file.
			if (unzOpenCurrentFile(zipfile) != UNZ_OK)
			{
				CLogE("can not open file %s", fileName);
				unzClose(zipfile);
				return false;
			}

			// Create a file to store current file.
			FILE *out = fopen(fullPath.c_str(), "wb");
			if (! out)
			{
				CLogE("can not open destination file %s", fullPath.c_str());
				unzCloseCurrentFile(zipfile);
				unzClose(zipfile);
				return false;
			}

			// Write current file content to destinate file.
			int error = UNZ_OK;
			do
			{
				error = unzReadCurrentFile(zipfile, readBuffer, ZIP_BUFFER_SIZE);
				if (error < 0)
				{
					CLogE("can not read zip file %s, error code is %d", fileName, error);
					unzCloseCurrentFile(zipfile);
					unzClose(zipfile);
					return false;
				}

				if (error > 0)
				{
					fwrite(readBuffer, error, 1, out);
				}
			} while(error > 0);

			fclose(out);
		}

		unzCloseCurrentFile(zipfile);

		// progress now
		if ( pnow )
		{
			*pnow = i;
		}

		// Goto next entry listed in the zip file.
		if ((i+1) < global_info.number_entry)
		{
			if (unzGoToNextFile(zipfile) != UNZ_OK)
			{
				CLogE("can not read next file");
				unzClose(zipfile);
				return false;
			}
		}
	}

	CLogD("end uncompressing");
	unzClose(zipfile);

	return true;
}

//int CUtils::decompress_fd(FILE* fin, FILE* fout)
//{
//	return inf(fin, fout);
//}

std::string CUtils::filehash_md5str(FILE* fp, char* buf, size_t buf_size, double* pnow /*= NULL*/, double* ptotal /*= NULL*/)
{
	if ( ptotal )
	{
		fseek(fp, 0, SEEK_END); 
		*ptotal = ftell(fp);
		fseek(fp, 0, SEEK_SET);
	}

	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];
	size_t file_size = 0;
	md5_init(&state);
	do
	{
		size_t readsize = fread(buf, 1, buf_size, fp);
		file_size += readsize;
		md5_append(&state, (const md5_byte_t *)buf, readsize);

		if ( pnow )
		{
			*pnow = file_size;
		}
	} while( !feof(fp) && !ferror(fp) );
	md5_finish(&state, digest);

	for (int di = 0; di < 16; ++di)
		sprintf(hex_output + di * 2, "%02x", digest[di]);

	hex_output[16*2] = 0;

	return std::string(hex_output);
}

// directory access
bool CUtils::access(const char* path, int mode)
{
	return ::access(path, mode) == 0;
}

// make directory
bool CUtils::mkdir(const char* path)
{
#if defined(_WIN32)
	return ::mkdir(path) == 0;
#else
	return ::mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == 0;
#endif
}


// build path directorys
bool CUtils::builddir(const char* path)
{
	std::string str(path);

	str_replace_ch(str, '\\', '/');

	size_t end = str.find_last_not_of('/');
	bool dummy = false;
	for ( size_t i = 1; // ignore root
		i < str.size(); i++ )
	{
		if ( str[i] == '/' && !dummy )
		{
			std::string bpath = str.substr(0, i);
			if ( !CUtils::access(bpath.c_str(), 0) )
			{
				if ( !CUtils::mkdir(bpath.c_str()) )
				{
					return false;
				}
			}
			dummy = true;
		}
		else
		{
			dummy = false;
		}
	}

	return true;
}

bool CUtils::rename(const char* from, const char* to)
{
	return 0 == ::rename(from, to);
}

// remove file
bool CUtils::remove(const char* path)
{
	return 0 == ::remove(path);
}

// replace char
size_t CUtils::str_replace_ch(std::string& str, char which, char to)
{
	size_t num = 0;
	for ( size_t i = 0; i < str.size(); i++ )
	{
		if ( str[i] == which )
		{
			str[i] = to;
			num++;
		}
	}
	return num;
}

std::string CUtils::str_trim(std::string s)
{
	if (s.length() == 0) return s;
	size_t beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
	size_t end = s.find_last_not_of(" \a\b\f\n\r\t\v");
	if (beg == std::string::npos) return "";
	return std::string(s, beg, end - beg + 1);
}


} /* namespace cells */
