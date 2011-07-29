/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Simple OS abstraction layer.
 */

#ifndef _OS_HPP_
#define _OS_HPP_

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <dlfcn.h>

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif
#define PATH_SEP '\\'
#else /* !WIN32 */
#define PATH_SEP '/'
#endif /* !WIN32 */

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

namespace OS {

void AcquireMutex(void);

void ReleaseMutex(void);

bool GetProcessName(char *str, size_t size);
bool GetCurrentDir(char *str, size_t size);

void DebugMessage(const char *format, ...);

/**
 * Get the current time in microseconds from an unknown base.
 */
long long GetTime(void);

void Abort(void);

} /* namespace OS */

#endif /* _OS_HPP_ */
