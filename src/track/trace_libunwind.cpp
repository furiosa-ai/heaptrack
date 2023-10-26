/*
    SPDX-FileCopyrightText: 2014-2019 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

/**
 * @brief A libunwind based backtrace.
 */

#include "trace.h"
#include "libheaptrack.h"

#include "util/libunwind_config.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <string.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

void Trace::print()
{
#if LIBUNWIND_HAS_UNW_GETCONTEXT && LIBUNWIND_HAS_UNW_INIT_LOCAL
    unw_context_t context;
    unw_getcontext(&context);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    int frameNr = 0;
    while (unw_step(&cursor)) {
        ++frameNr;
        unw_word_t ip = 0;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);

        unw_word_t sp = 0;
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        char symbol[256] = {"<unknown>"};
        unw_word_t offset = 0;
        unw_get_proc_name(&cursor, symbol, sizeof(symbol), &offset);

        fprintf(stderr, "#%-2d 0x%016" PRIxPTR " sp=0x%016" PRIxPTR " %s + 0x%" PRIxPTR "\n", frameNr,
                static_cast<uintptr_t>(ip), static_cast<uintptr_t>(sp), symbol, static_cast<uintptr_t>(offset));
    }
#endif
}

bool Trace::isSomeProcListed()
{
    if (!HtExtUtil().isFunctionNameBlacklistSet()) {
        return false;
    }
#if LIBUNWIND_HAS_UNW_GETCONTEXT && LIBUNWIND_HAS_UNW_INIT_LOCAL
    unw_context_t context; unw_cursor_t cursor;
    unw_word_t offset;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    char buf[HEAPTRACK_PRELOAD_FUNCTION_NAME_BLACKLIST_MAX_NAME_LENGTH];
    size_t minLengthToSearch = HtExtUtil().minFunctionNameBufferLengthToSearch();
    while (unw_step(&cursor)) {
        unw_get_proc_name(&cursor, buf, minLengthToSearch, &offset);
        if (HtExtUtil().isIgnoredByFuncName(buf)) {
            return true;
        }
    }
    return false;
#endif
}

void Trace::setup()
{
    // configure libunwind for better speed
    if (unw_set_caching_policy(unw_local_addr_space, UNW_CACHE_PER_THREAD)) {
        fprintf(stderr, "WARNING: Failed to enable per-thread libunwind caching.\n");
    }
#if LIBUNWIND_HAS_UNW_SET_CACHE_SIZE
    if (unw_set_cache_size(unw_local_addr_space, 1024, 0)) {
        fprintf(stderr, "WARNING: Failed to set libunwind cache size.\n");
    }
#endif
}

int Trace::unwind(void** data)
{
    return unw_backtrace(data, MAX_SIZE);
}
