#include "observer/Log/log_stacktrace.hpp"

/**
 * Simplified version of palanteer.h
 */

#include <csignal>
#include <cstdlib>
#include <cstring>

#include "observer/Log/log.hpp"

#ifndef PL_DYN_STRING_MAX_SIZE
#define PL_DYN_STRING_MAX_SIZE 512
#endif

static obs_sig_hndl oldSigHandlers[7] = {0};

#if PL_IMPL_STACKTRACE == 0
void ObserverLogStackTrace(void) {
    OBSERVER_ERROR("Stacktrace not implemented for this platform");
}
#endif

void ObserverRegisterSignalHandlers(obs_sig_hndl newHandler) {
    memset(oldSigHandlers, 0, sizeof(oldSigHandlers));

    // Register the signal handlers
    oldSigHandlers[0] = std::signal(SIGABRT, newHandler);
    oldSigHandlers[1] = std::signal(SIGFPE, newHandler);
    oldSigHandlers[2] = std::signal(SIGILL, newHandler);
    oldSigHandlers[3] = std::signal(SIGSEGV, newHandler);

    // oldSigHandlers[4] = std::signal(SIGINT, newHandler);

    oldSigHandlers[5] = std::signal(SIGTERM, newHandler);
#if defined(__unix__)
    oldSigHandlers[6] = std::signal(SIGPIPE, newHandler);
#endif
    // NOTE: on windows you should register AddVectoredExceptionHandler
}
void ObserverCallOldSignalHandlers(int signal) {
    if (oldSigHandlers[0]) std::signal(SIGABRT, oldSigHandlers[0]);
    if (oldSigHandlers[1]) std::signal(SIGFPE, oldSigHandlers[1]);
    if (oldSigHandlers[2]) std::signal(SIGILL, oldSigHandlers[2]);
    if (oldSigHandlers[3]) std::signal(SIGSEGV, oldSigHandlers[3]);
    // if (oldSigHandlers[4]) std::signal(SIGINT, oldSigHandlers[4]);
    if (oldSigHandlers[5]) std::signal(SIGTERM, oldSigHandlers[5]);
#if defined(__unix__)
    if (oldSigHandlers[6]) std::signal(SIGPIPE, oldSigHandlers[6]);
#endif
    // std::raise(signal);
    switch (signal) {
        case SIGABRT:
            oldSigHandlers[0](signal);
            break;
        case SIGFPE:
            oldSigHandlers[1](signal);
            break;
        case SIGILL:
            oldSigHandlers[2](signal);
            break;
        case SIGSEGV:
            oldSigHandlers[3](signal);
            break;
        // case SIGINT:
        //     oldSigHandlers[4](signal);
        //     break;
        case SIGTERM:
            oldSigHandlers[5](signal);
            break;
#if defined(__unix__)
        case SIGPIPE:
            oldSigHandlers[6](signal);
            break;
#endif
        default:
            // Handle unexpected signals if necessary
            break;
    }
}

#if defined(__unix__) && PL_IMPL_STACKTRACE == 1

#include <cxxabi.h>
#include <elfutils/libdwfl.h>
#include <unistd.h>

#include "libunwind.h"

void ObserverLogStackTrace(void) {
    // Initialize libunwind
    unw_context_t uc;
    unw_getcontext(&uc);
    unw_cursor_t cursor;
    unw_init_local(&cursor, &uc);
    char localMsgStr[PL_DYN_STRING_MAX_SIZE];
    unw_word_t offset;
    unw_word_t ip;

    // Initialize DWARF reading
    char* debugInfoPath = NULL;
    Dwfl_Callbacks callbacks = {};
    callbacks.find_elf = dwfl_linux_proc_find_elf;
    callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
    callbacks.debuginfo_path = &debugInfoPath;
    Dwfl* dwfl = dwfl_begin(&callbacks);
    if (!dwfl || dwfl_linux_proc_report(dwfl, getpid()) != 0 ||
        dwfl_report_end(dwfl, NULL, NULL) != 0) {
        return;
    }

    OBSERVER_ERROR("CRASH: Stacktrace");

    const int skipDepthQty = 2;  // No need to display the bottom machinery
    int depth = 0;
    // Loop on stack depth
    while (unw_step(&cursor) > 0) {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);

        if (depth >= skipDepthQty) {
            Dwarf_Addr addr = (uintptr_t)(ip - 4);
            Dwfl_Module* module = dwfl_addrmodule(dwfl, addr);
            Dwfl_Line* line = dwfl_getsrc(dwfl, addr);

            if (line) {
                Dwarf_Addr addr2;
                int lineNbr;
                int status;
                const char* filename =
                    dwfl_lineinfo(line, &addr2, &lineNbr, NULL, NULL, NULL);
                char* demangledName = abi::__cxa_demangle(
                    dwfl_module_addrname(module, addr), 0, 0, &status);
                // Filename and line first in the potentially truncated remote
                // log (demangled function name may be long)
                snprintf(localMsgStr, sizeof(localMsgStr),
                         "  #%-2d %s(%d) : %s", depth - skipDepthQty,
                         filename ? strrchr(filename, '/') + 1 : "<unknown>",
                         filename ? lineNbr : 0,
                         status ? dwfl_module_addrname(module, addr)
                                : demangledName);
                if (status == 0) free(demangledName);
            } else {
                snprintf(localMsgStr, sizeof(localMsgStr),
                         "  #%-2d 0x%" PRIX64 " : %s", depth - skipDepthQty,
                         ip - 4, dwfl_module_addrname(module, addr));
            }

            // Log
            OBSERVER_ERROR("{}", localMsgStr);
        }

        // Next unwinding
        localMsgStr[0] = 0;
        unw_get_proc_name(&cursor, localMsgStr, sizeof(localMsgStr),
                          &offset);  // Fails if there is no debug symbols
        if (!strcmp(localMsgStr, "main")) break;
        ++depth;
    }  // End of unwinding

    // End session
    dwfl_end(dwfl);
}
#endif  // if defined(__unix__) && PL_IMPL_STACKTRACE==1

#if defined(_WIN32) && PL_IMPL_STACKTRACE == 1
void crashLogStackTrace(void) {
    plScope("CRASH Stacktrace");
    char msgStr[PL_DYN_STRING_MAX_SIZE];
    char localMsgStr[512];
    char tmpStr[32];
    char depthStr[8];

    // Get the addresses of the stacktrace
    PVOID stacktrace[64];  // 64 levels of depth should be enough for everyone
    int foundStackDepth = implCtx.rtlWalkFrameChain
                              ? implCtx.rtlWalkFrameChain(stacktrace, 64, 0)
                              : 0;

    // Some required windows structures for the used APIs
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement = 0;
    constexpr int MaxNameSize = 8192;
    char symBuffer[sizeof(SYMBOL_INFO) + MaxNameSize];
    SYMBOL_INFO* symInfo = (SYMBOL_INFO*)symBuffer;
    symInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    symInfo->MaxNameLen = MaxNameSize;
    HANDLE proc = GetCurrentProcess();

#define PL_CRASH_STACKTRACE_DUMP_INFO_(itemNbrStr, colorItem, colorFunc,       \
                                       colorNeutral)                           \
    if (isFuncValid || isLineValid) {                                          \
        snprintf(tmpStr, sizeof(tmpStr), "(%u)",                               \
                 isLineValid ? line.LineNumber : 0);                           \
        snprintf(msgStr, PL_DYN_STRING_MAX_SIZE, "%s %s%s : %s", itemNbrStr,   \
                 isLineValid ? strrchr(line.FileName, '\\') + 1 : "<unknown>", \
                 isLineValid ? tmpStr : "",                                    \
                 isFuncValid ? symInfo->Name : "<unknown>");                   \
        snprintf(localMsgStr, sizeof(localMsgStr),                             \
                 "   " colorItem "%s " colorNeutral "%s%s : " colorFunc        \
                 "%s" colorNeutral "\n",                                       \
                 itemNbrStr,                                                   \
                 isLineValid ? strrchr(line.FileName, '\\') + 1 : "<unknown>", \
                 isLineValid ? tmpStr : "",                                    \
                 isFuncValid ? symInfo->Name : "<unknown>");                   \
    } else {                                                                   \
        snprintf(msgStr, PL_DYN_STRING_MAX_SIZE, "%s 0x%" PRIX64, itemNbrStr,  \
                 ptr);                                                         \
        snprintf(localMsgStr, sizeof(localMsgStr),                             \
                 "   " colorItem "%s" colorFunc " 0x%" PRIX64 colorNeutral     \
                 "\n",                                                         \
                 itemNbrStr, ptr);                                             \
    }                                                                          \
    plData("CRASH", msgStr);                                                   \
    PL_IMPL_PRINT_STDERR(localMsgStr, true, false)

    const int skipDepthQty = 3;  // No need to display the bottom machinery
    for (int depth = skipDepthQty; depth < foundStackDepth; ++depth) {
        uint64_t ptr = ((uint64_t)stacktrace[depth]) -
                       1;  // -1 because the captured PC is already pointing on
                           // the next code line at snapshot time

        // Get the nested inline function calls, if any
        DWORD frameIdx, curContext = 0;
        int inlineQty = SymAddrIncludeInlineTrace(proc, ptr);
        if (inlineQty > 0 && SymQueryInlineTrace(proc, ptr, 0, ptr, ptr,
                                                 &curContext, &frameIdx)) {
            for (int i = 0; i < inlineQty; ++i) {
                bool isFuncValid = (SymFromInlineContext(proc, ptr, curContext,
                                                         0, symInfo) != 0);
                bool isLineValid =
                    (SymGetLineFromInlineContext(proc, ptr, curContext, 0,
                                                 &displacement, &line) != 0);
                ++curContext;
#if PL_IMPL_CONSOLE_COLOR == 1
                PL_CRASH_STACKTRACE_DUMP_INFO_("inl", "\033[93m", "\033[36m",
                                               "\033[0m");
#else
                PL_CRASH_STACKTRACE_DUMP_INFO_("inl", "", "", "");
#endif
            }
        }

        // Get the function call for this depth
        bool isFuncValid = (SymFromAddr(proc, ptr, 0, symInfo) != 0);
        bool isLineValid =
            (SymGetLineFromAddr64(proc, ptr - 1, &displacement, &line) != 0);
        snprintf(depthStr, sizeof(depthStr), "#%-2d", depth - skipDepthQty);
#if PL_IMPL_CONSOLE_COLOR == 1
        PL_CRASH_STACKTRACE_DUMP_INFO_(depthStr, "\033[93m", "\033[36m",
                                       "\033[0m");
#else
        PL_CRASH_STACKTRACE_DUMP_INFO_(depthStr, "", "", "");
#endif
    }  // End of loop on stack depth
}
#endif  // if defined(_WIN32) && PL_IMPL_STACKTRACE==1
