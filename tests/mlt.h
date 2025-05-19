/* Copyright (C) 2015 Alexandre Mulatinho <alex@mulatinho.net>.
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifndef MLT_TESTING
#define MLT_TESTING 1
#endif

/* ANSI Color codes */
#define MLT_COLOR_RED     "\x1b[31m"
#define MLT_COLOR_GREEN   "\x1b[32m"
#define MLT_COLOR_YELLOW  "\x1b[33m"
#define MLT_COLOR_BOLD    "\x1b[1m"
#define MLT_COLOR_RESET   "\x1b[0m"

/* Test suite support */
#define MLT_MAX_SUITE_NAME 64
#define MLT_MAX_SUITES 16

struct mlt_suite {
	char name[MLT_MAX_SUITE_NAME];
	int tests;
	int success;
	double time_elapsed;
};

static struct mlt_suite mlt_suites[MLT_MAX_SUITES];
static int mlt_current_suite_idx = -1;
static int mlt_total_suites = 0;

static int mlt_rsuccess = 0;
static int mlt_rtests = 0;
static int mlt_result = 0;

static struct timeval mlt_init_t, mlt_subinit_t, mlt_end_t, mlt_subend_t;

#define mlt_debug(...) \
	fprintf(stderr, __VA_ARGS__);

#define mlt_time_calc(end, ini) \
	(((end.tv_sec - ini.tv_sec) * 1000000UL + end.tv_usec) - ini.tv_usec) / 1000000.0

#define mlt_start() do { \
	gettimeofday(&mlt_init_t, NULL); \
	fprintf(stdout, MLT_COLOR_BOLD ":. Starting test session at %s" MLT_COLOR_RESET, \
		ctime((const time_t *) &mlt_init_t.tv_sec)); \
	mlt_rsuccess = 0; mlt_rtests = 0; mlt_result = 0; \
	mlt_total_suites = 0; mlt_current_suite_idx = -1; \
	} while(0)

#define mlt_finish() do { \
	gettimeofday(&mlt_end_t, NULL); \
	mlt_result = mlt_rtests - mlt_rsuccess; \
	printf("\n" MLT_COLOR_BOLD "=== Test Session Summary ===" MLT_COLOR_RESET "\n"); \
	printf("Total time elapsed: %.3fs\n", mlt_time_calc(mlt_end_t, mlt_init_t)); \
	for (int i = 0; i < mlt_total_suites; i++) { \
		printf(MLT_COLOR_YELLOW "Suite: %s" MLT_COLOR_RESET "\n", mlt_suites[i].name); \
		printf("  Tests: %d, " MLT_COLOR_GREEN "Passed: %d" MLT_COLOR_RESET ", " \
			MLT_COLOR_RED "Failed: %d" MLT_COLOR_RESET " (%.3fs)\n", \
			mlt_suites[i].tests, mlt_suites[i].success, \
			(mlt_suites[i].tests - mlt_suites[i].success), \
			mlt_suites[i].time_elapsed); \
	} \
	printf("\n" MLT_COLOR_BOLD "=== Final Results ===" MLT_COLOR_RESET "\n"); \
	if (!(mlt_result)) \
		printf(MLT_COLOR_GREEN "PASSED" MLT_COLOR_RESET ": All tests completed successfully\n"); \
	else \
		printf(MLT_COLOR_RED "FAILED" MLT_COLOR_RESET ": Some tests did not pass\n"); \
	printf("Total Tests: %d, " MLT_COLOR_GREEN "Passed: %d" MLT_COLOR_RESET ", " \
		MLT_COLOR_RED "Failed: %d" MLT_COLOR_RESET "\n\n", \
		mlt_rtests, mlt_rsuccess, (mlt_rtests-mlt_rsuccess)); \
	return mlt_result; \
	} while(0)

#define mlt_suite_begin(suite_name) do { \
	if (mlt_total_suites >= MLT_MAX_SUITES) { \
		fprintf(stderr, "Error: Maximum number of test suites exceeded\n"); \
		exit(1); \
	} \
	mlt_current_suite_idx = mlt_total_suites++; \
	memset(mlt_suites[mlt_current_suite_idx].name, '\0', MLT_MAX_SUITE_NAME); \
	snprintf(mlt_suites[mlt_current_suite_idx].name, MLT_MAX_SUITE_NAME - 1, "%s", suite_name); \
	mlt_suites[mlt_current_suite_idx].tests = 0; \
	mlt_suites[mlt_current_suite_idx].success = 0; \
	gettimeofday(&mlt_subinit_t, NULL); \
	printf(MLT_COLOR_YELLOW "\n- Running suite: %s" MLT_COLOR_RESET "\n", suite_name); \
	} while(0)

#define mlt_suite_end() do { \
	gettimeofday(&mlt_subend_t, NULL); \
	mlt_suites[mlt_current_suite_idx].time_elapsed = mlt_time_calc(mlt_subend_t, mlt_subinit_t); \
	} while(0)

#define mlt_show_result(res) \
	printf("  %s %s" MLT_COLOR_RESET " in '%s()' line %d: %s\n", \
	res ? MLT_COLOR_GREEN "[ OK]" : MLT_COLOR_RED "[ERR]", \
	res ? "PASS" : "FAIL", \
	__func__, __LINE__, #res)

#define mlt_assert(test) do { \
	if (test) { \
		mlt_rsuccess++; \
		mlt_suites[mlt_current_suite_idx].success++; \
	} \
	mlt_rtests++; \
	mlt_suites[mlt_current_suite_idx].tests++; \
	mlt_show_result(test); \
	} while(0)

#define mlt_time_init() gettimeofday(&mlt_subinit_t, NULL)

#define mlt_streq(in, str) mlt_assert(strncmp(in, str, strlen(in)) == 0)
#define mlt_strneq(in, str) mlt_assert(strncmp(in, str, strlen(in)))
