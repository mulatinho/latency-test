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
#include <ctype.h>
#include <regex.h>
#include <errno.h>

#ifndef MLT_H
#define MLT_TESTING 1

/* ANSI Color codes */
#define M_CLR_RED     "\033[31m"
#define M_CLR_GREEN   "\033[32m"
#define M_CLR_YELLOW  "\033[33m"
#define M_CLR_BOLD    "\033[1m"
#define M_CLR_RESET   "\033[0m"

/* Test suite support */
#define MLT_MAX_SUITE_NAME 64
#define MLT_MAX_SUITES 16

/* Struct where the Suites are defined */
struct mlt_suite {
	char name[MLT_MAX_SUITE_NAME];
	int tests;
	int success;
	double time_elapsed;
};

/* Internal global variables */
static struct mlt_suite mlt_suites[MLT_MAX_SUITES];
static int mlt_current_suite_idx = -1;
static int mlt_total_suites = 0;

static int mlt_rsuccess = 0;
static int mlt_rtests = 0;
static int mlt_result = 0;

static struct timeval mlt_init_t = {0}, mlt_end_t = {0}, mlt_subinit_t = {0}, mlt_subend_t = {0};

#define mlt_debug(...) fprintf(stderr, __VA_ARGS__);

#define mlt_time_init() gettimeofday(&mlt_subinit_t, NULL)

#define mlt_time_calc(end, ini) \
	(((end.tv_sec - ini.tv_sec) * 1000000UL + end.tv_usec) - ini.tv_usec) / 1000000.0

#define mlt_start() do { \
	gettimeofday(&mlt_init_t, NULL); \
	fprintf(stdout, M_CLR_BOLD ":. Starting MLT Test Session\n" M_CLR_RESET); \
	mlt_rsuccess = 0; mlt_rtests = 0; mlt_result = 0; \
	mlt_total_suites = 0; mlt_current_suite_idx = -1; \
	} while(0)

#define mlt_finish() do { \
	gettimeofday(&mlt_end_t, NULL); \
	mlt_result = mlt_rtests - mlt_rsuccess; \
	printf("\n" M_CLR_BOLD "=== Test Session Summary ===" M_CLR_RESET "\n"); \
	printf("Total time elapsed: %.3fs\n", mlt_time_calc(mlt_end_t, mlt_init_t)); \
	for (int i = 0; i < mlt_total_suites; i++) { \
		printf(M_CLR_YELLOW "Suite: %s" M_CLR_RESET "\n", mlt_suites[i].name); \
		printf("  Tests: %d, " M_CLR_GREEN "Passed: %d" M_CLR_RESET ", " \
			M_CLR_RED "Failed: %d" M_CLR_RESET " (%.3fs)\n", \
			mlt_suites[i].tests, mlt_suites[i].success, \
			(mlt_suites[i].tests - mlt_suites[i].success), \
			mlt_suites[i].time_elapsed); \
	} \
	printf("\n" M_CLR_BOLD "=== Final Results ===" M_CLR_RESET "\n"); \
	if (!(mlt_result)) \
		printf(M_CLR_GREEN "PASSED" M_CLR_RESET ": All tests completed successfully\n"); \
	else \
		printf(M_CLR_RED "FAILED" M_CLR_RESET ": Some tests did not pass\n"); \
	printf("Total Tests: %d, " M_CLR_GREEN "Passed: %d" M_CLR_RESET ", " \
		M_CLR_RED "Failed: %d" M_CLR_RESET "\n\n", \
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
	printf(M_CLR_YELLOW "\n- Running suite: %s" M_CLR_RESET "\n", suite_name); \
	} while(0)

#define mlt_suite_end() do { \
	gettimeofday(&mlt_subend_t, NULL); \
	mlt_suites[mlt_current_suite_idx].time_elapsed = mlt_time_calc(mlt_subend_t, mlt_subinit_t); \
	} while(0)

#define mlt_show_result(res) \
	printf("  %s %s" M_CLR_RESET " in '%s()' line %d: %s\n", \
	res ? M_CLR_GREEN "[ OK]" : M_CLR_RED "[ERR]", \
	res ? "PASS" : "FAIL", \
	__func__, __LINE__, #res)

/* Common Utilities */
#define mlt_assert(test) do { \
	if (test) { \
		mlt_rsuccess++; \
		mlt_suites[mlt_current_suite_idx].success++; \
	} \
	mlt_rtests++; \
	mlt_suites[mlt_current_suite_idx].tests++; \
	mlt_show_result(test); \
	} while(0)

#define mlt_streq(in, str) mlt_assert(strncmp(in, str, strlen(in)) == 0)

#define mlt_strneq(in, str) mlt_assert(strncmp(in, str, strlen(in)))

/* String Utilities */
#define mlt_strupper(str) do { \
    for (char *p = (str); *p; ++p) *p = toupper(*p); \
} while (0)

#define mlt_strlower(str) do { \
    for (char *p = (str); *p; ++p) *p = tolower(*p); \
} while (0)

#define mlt_strcontains(str, substr) (strstr((str), (substr)) != NULL ? 1 : 0)

#define mlt_strpaste(a, b) a ## b

#define mlt_stringify(a) #a

/* Array uttlities */
#define mlt_arr_len(arr) (sizeof(arr)/sizeof((arr)[0]))

#define mlt_print_int_arr(arr, n) for (size_t i = 0; i < n; i++) printf("%d ", arr[i])

#define mlt_print_float_arr(arr, n) for (size_t i = 0; i < n; i++) printf("%.2f ", arr[i])

#define mlt_print_double_arr(arr, n) for (size_t i = 0; i < n; i++) printf("%.2f ", arr[i])

#define mlt_print_str_arr(arr, n) for (size_t i = 0; i < n; i++) printf("%s\n", arr[i])

/* Sort Utilities */
#define mlt_define_comparision(type, name, expr) \
    int mlt_name(const void *a, const void *b) { \
        type mlt__a = *(const type *)a, _b = *(const type *)b; \
        return mlt_(expr); \
    }
    
#define mlt_define_compare_str(name) \
    int mlt_name(const void *a, const void *b) { \
        return mlt_strcmp(*(const char **)a, *(const char **)b); \
    }

#define mlt_define_compare_struct(type, field, name) \
    int mlt_name(const void *a, const void *b) { \
        return mlt_((const type *)a)->field - ((const type *)b)->field; \
    }

/* Regex Utilities */
#define mlt_regex_compile(pattern, regex_ptr) \
    regcomp(regex_ptr, pattern, REG_EXTENDED)

#define mlt_regex_exec(regex_ptr, text) \
    regexec(regex_ptr, text, 0, NULL, 0)

#define mlt_regex_exec_match(regex_ptr, text, nmatch, pmatch) \
    regexec(regex_ptr, text, nmatch, pmatch, 0)

#define mlt_regex_free(regex_ptr) \
    regfree(regex_ptr)

#endif
