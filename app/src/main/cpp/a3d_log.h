/*
 * Copyright (c) 2009-2010 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef a3d_log_H
#define a3d_log_H

/***********************************************************
* Macros are based on Android's logging mechanism found in *
* system/core/include/cutils/log.h                         *
*                                                          *
* Before including this file declare LOG_TAG as follows:   *
* #define LOG_TAG "tag"                                    *
*                                                          *
* Declare LOG_DEBUG before including a3d_log.h to enable   *
* debugging.                                               *
*                                                          *
* LOG{DIWE}("") will output func@line with no message      *
***********************************************************/

// support non-Android environments
#ifdef ANDROID
	#include <android/log.h>
#else
	#define ANDROID_LOG_DEBUG 0
	#define ANDROID_LOG_INFO  1
	#define ANDROID_LOG_WARN  2
	#define ANDROID_LOG_ERROR 3
#endif

#ifndef LOG_TAG
	#define LOG_TAG NULL
#endif

// logging using Android "standard" macros
void a3d_log(const char* func, int line, int type, const char* tag, const char* fmt, ...);

// tracing using Android Systrace
void a3d_trace_init(void);
void a3d_trace_begin(const char* func, int line);
void a3d_trace_end(void);

#ifndef LOGD
	#ifdef LOG_DEBUG
		#define LOGD(...) (a3d_log(__func__, __LINE__, ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
	#else
		#define LOGD(...)
	#endif
#endif

#ifndef LOGI
	#define LOGI(...) (a3d_log(__func__, __LINE__, ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGW
	#define LOGW(...) (a3d_log(__func__, __LINE__, ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGE
	#define LOGE(...) (a3d_log(__func__, __LINE__, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#ifndef TRACE_INIT
	#ifdef TRACE_DEBUG
		#define TRACE_INIT() (a3d_trace_init())
	#else
		#define TRACE_INIT()
	#endif
#endif

#ifndef TRACE_BEGIN
	#ifdef TRACE_DEBUG
		#define TRACE_BEGIN() (a3d_trace_begin(__func__, __LINE__))
	#else
		#define TRACE_BEGIN()
	#endif
#endif

#ifndef TRACE_END
	#ifdef TRACE_DEBUG
		#define TRACE_END() (a3d_trace_end())
	#else
		#define TRACE_END()
	#endif
#endif

#endif
