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

#ifndef CPLATFORM_H_
#define CPLATFORM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

#if defined(_WIN32)
#	include <windows.h>
#	include <direct.h>
#	include <io.h>
#else // __linux__
#	include <unistd.h>
#	include <pthread.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <sys/time.h>
#endif

// if using cocos2dx
#define USING_COCOS2DX 1

#if USING_COCOS2DX
#	include <platform/CCCommon.h>
#	define PRINT_LOG cocos2d::CCLog
#else
#	define PRINT_LOG printf
#endif

// log interface
#ifdef _DUMP_DEBUG
#define CLogD(format, ...)      PRINT_LOG(format, ##__VA_ARGS__)	// log dump debug
#define CLogI(format, ...)      PRINT_LOG(format, ##__VA_ARGS__)	// log info
#define CLogE(format, ...)      PRINT_LOG(format, ##__VA_ARGS__)	// log error
#define CLog(format, ...)		CLogI(format, ##__VA_ARGS__)	// shortcut for log info
#elif defined(_DEBUG) || defined(_DUMP_LOG)
#define CLogD(format, ...)
#define CLogI(format, ...)      PRINT_LOG(format, ##__VA_ARGS__)
#define CLogE(format, ...)      PRINT_LOG(format, ##__VA_ARGS__)
#define CLog(format, ...)		CLogI(format, ##__VA_ARGS__)
#else // release
#define CLogD(format, ...) 
#define CLogI(format, ...)		PRINT_LOG(format, ##__VA_ARGS__)
#define CLogE(format, ...)      PRINT_LOG(format, ##__VA_ARGS__)
#define CLog(format, ...)		CLogI(format, ##__VA_ARGS__)
#endif

#if defined(_WIN32)

// struct for gettimeofday
struct timeval_t
{
	long    tv_sec;        // seconds
	long    tv_usec;    // microSeconds
};
#else // __linux__

typedef timeval timeval_t;

#endif // defined(_WIN32)



#endif /* CPLATFORM_H_ */
