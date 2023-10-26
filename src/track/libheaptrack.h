/*
    SPDX-FileCopyrightText: 2014-2017 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>

#define HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_NAME_LENGTH 1024
#define HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_LIST_SIZE 256

#ifdef __cplusplus
typedef class LineWriter linewriter_t;
extern "C" {
#else
typedef struct LineWriter linewriter_t;
#endif

typedef void (*heaptrack_callback_t)();
typedef void (*heaptrack_callback_initialized_t)(linewriter_t&);

typedef const char *str_ro, *str_ro_probe;

struct FunctionName {
    str_ro name = nullptr;
    size_t length = 0;
};

class FunctionNameBlacklist
{
public:
    enum AddResult {
        Failure_WasFull,
        Failure_TooLong,
        Success_NotFullYet,
        Success_FullNow,
    };

    // `name`: non-null
    AddResult add(str_ro name, size_t length) {
        if (size >= HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_LIST_SIZE) {
            return Failure_WasFull;
        }
        if (length > HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_NAME_LENGTH) {
            return Failure_TooLong;
        }
        list[size].name = name;
        list[size].length = length;
        size++;
        if (size >= HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_LIST_SIZE) {
            return Success_FullNow;
        } else {
            return Success_NotFullYet;
        }
    }

    bool isEmpty() {
        return size == 0;
    }

    // `name`: non-null
    bool contains(str_ro name) {
        size_t length = strlen(name);
        for (size_t i = 0; i < size; i++) {
            if (list[i].length == length) {
                if (strncmp(list[i].name, name, length) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    size_t maxLengthPlusTwo = 2;

private:
    size_t size = 0;
    FunctionName list[HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_LIST_SIZE];
};

class HtExt
{
public:
    void setAllocSizeThreshold(str_ro env) {
        if (env == nullptr) {
            threshold = 0;
        } else {
            threshold = atoi(env);
        }
    }

    void setFunctionNameBlacklist(str_ro env) {
        if (env == nullptr) {
            return;
        }
        size_t maxLength = 0;
        str_ro_probe probe = env;
        str_ro name = probe;
        while (true) {
            if (
                *probe == ':' || *probe == ' ' ||
                *probe == '\n' || *probe == '\0'
            ) {
                if (probe != name) {
                    size_t length = probe - name;
                    FunctionNameBlacklist::AddResult addResult = blacklist.add(name, length);
                    if (addResult == FunctionNameBlacklist::AddResult::Failure_WasFull) {
                        break;
                    }
                    if (addResult != FunctionNameBlacklist::AddResult::Failure_TooLong) {
                        if (length > maxLength) {
                            maxLength = length;
                        }
                        if (addResult == FunctionNameBlacklist::AddResult::Success_FullNow) {
                            break;
                        }
                    }
                }
                name = probe + 1;
                if (*probe == '\0') {
                    break;
                }
            }
            probe++;
        }
        blacklist.maxLengthPlusTwo = maxLength + 2;
    }

    bool isIgnoredByAllocSize(size_t size) {
        if (threshold == 0) {
            return false;
        } else {
            return size < threshold;
        }
    }

    bool isFunctionNameBlacklistSet() {
        return !blacklist.isEmpty();
    }

    size_t minFunctionNameBufferLengthToSearch() {
        return blacklist.maxLengthPlusTwo;
    }

    bool isIgnoredByFuncName(str_ro name) {
        return blacklist.contains(name);
    }

private:
    size_t threshold = 0;
    FunctionNameBlacklist blacklist;
};

__attribute__((weak)) HtExt& HtExtUtil()
{
    static HtExt config;
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
