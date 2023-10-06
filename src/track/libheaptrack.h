/*
    SPDX-FileCopyrightText: 2014-2017 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define HEAPTRACK_FUNCTION_NAME_FILTER_MAX_SIZE 256

#ifdef __cplusplus
typedef class LineWriter linewriter_t;
extern "C" {
#else
typedef struct LineWriter linewriter_t;
#endif

typedef void (*heaptrack_callback_t)();
typedef void (*heaptrack_callback_initialized_t)(linewriter_t&);

struct Ht3rdConfig
{
    uint64_t allocSizeThreshold = 0;
    unsigned int numFuncName = 0;
    unsigned int maxLengthPlusTwo = 2;
    const char *funcNameStrings[HEAPTRACK_FUNCTION_NAME_FILTER_MAX_SIZE] = {nullptr};
    unsigned int funcNameLengths[HEAPTRACK_FUNCTION_NAME_FILTER_MAX_SIZE] = {0};
};

__attribute__((weak)) Ht3rdConfig& ht3config()
{
    static Ht3rdConfig config;
    return config;
}

void heaptrack_init(
    const char* outputFileName,
    heaptrack_callback_t initCallbackBefore,
    heaptrack_callback_initialized_t initCallbackAfter,
    heaptrack_callback_t stopCallback
);

void heaptrack_stop();

void heaptrack_pause();

void heaptrack_resume();

void heaptrack_malloc(void* ptr, size_t size);

void heaptrack_free(void* ptr);

void heaptrack_realloc(void* ptr_in, size_t size, void* ptr_out);
void heaptrack_realloc2(uintptr_t ptr_in, size_t size, uintptr_t ptr_out);

void heaptrack_invalidate_module_cache();

typedef void (*heaptrack_warning_callback_t)(FILE*);
void heaptrack_warning(heaptrack_warning_callback_t callback);

#ifdef __cplusplus
}
#endif
