/* Copyright (C) 2015 Alexandre Mulatinho <alex at mulatinho.net>.
   mlt.h is a simple group of directives to test source files in C.  

   This file is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The 'mlt.h' file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#ifndef MLT_TESTING
#define MLT_TESTING 1
#endif

int mlt_rsuccess = 0;
int mlt_rtests = 0;
int mlt_result = 0;

struct timeval mlt_init_t, mlt_subinit_t, mlt_end_t, mlt_subend_t;

#define mlt_time_calc(end, ini) \
	(((end.tv_sec - ini.tv_sec) * 1000000UL + end.tv_usec) - ini.tv_usec) / 1000000.0

#define mlt_start() do { \
	gettimeofday(&mlt_init_t, NULL); \
	fprintf(stdout, ":. Started test(s) on '%s' at %s\n", \
		__FILE__, ctime((const time_t *) &mlt_init_t.tv_sec)); \
	mlt_rsuccess = 0; mlt_rtests = 0; mlt_result = 0; \
	} while(0)

#define mlt_finish() do { \
	gettimeofday(&mlt_end_t, NULL); \
	mlt_result = mlt_rtests - mlt_rsuccess; \
	if (!(mlt_result)) \
		printf("\n:. Result: PASSED, Time Elapsed: %.3fms, Filename: '%s'\n", \
			mlt_time_calc(mlt_end_t, mlt_init_t), __FILE__); \
	else printf("\n:. Result: FAILED, Time Elapsed: %.3fms, Filename: '%s'\n", \
			mlt_time_calc(mlt_end_t, mlt_init_t), __FILE__); \
	printf(":. Tests run: %d, Tests PASSED: %d, Tests FAILED: %d\n\n", \
		mlt_rtests, mlt_rsuccess, (mlt_rtests-mlt_rsuccess)); \
	return mlt_result; \
	} while(0)

#define mlt_debug(res) \
	printf("return %s in '%s' on function '%s()' line %d,\ntest '%s'\n", \
	res ? "success" : "error  ", __FILE__, __func__, __LINE__, __STRING(res))

#define mlt_time_end() do { \
	gettimeofday(&mlt_subend_t, NULL); \
	printf("  elapsed time in '%s' on function '%s()', time: %.3fms\n", \
		__FILE__, __func__, mlt_time_calc(mlt_subend_t, mlt_subinit_t)); \
	} while(0)

#define mlt_assert(test) do { \
	if (test) { mlt_rsuccess++; } \
	mlt_rtests++; mlt_debug(test); \
	} while(0)

#define mlt_time_init() gettimeofday(&mlt_subinit_t, NULL);

#define mlt_streq(in, str) mlt_assert(strncmp(in, str, strlen(in)) == 0);
#define mlt_strneq(in, str) mlt_assert(strncmp(in, str, strlen(in)));
