#pragma once

#if defined(OBSERVER_ENABLE_INSTRUMENTATION) && OBSERVER_ENABLE_INSTRUMENTATION

// #ifdef OBSERVER_USE_PALANTEER
// NOTE: Write #define PL_IMPLEMENTATION 1 before including this file!

#include "palanteer.h"

#if OBSERVER_PALANTEER_CONNECTED
#define OBSERVER_INIT_INSTRUMENTATION plInitAndStart("Observer")
#else
#define OBSERVER_INIT_INSTRUMENTATION() \
    plSetFilename("observer.pltraw"),   \
        plInitAndStart("Observer", PL_MODE_STORE_IN_FILE)

#define OBSERVER_STOP_INSTRUMENTATION() plStopAndUninit();
#endif

#define OBSERVER_SCOPE(name) plScope(name)
#define OBSERVER_SCOPE_BEGIN(name) plBegin(name)
#define OBSERVER_SCOPE_END(name) plEnd(name)

#define OBSERVER_DECLARE_THREAD(name) plDeclareThread(name)

// #elif defined(OBSERVER_USE_OPENTELEMETRY)

#else
#define OBSERVER_INIT_INSTRUMENTATION()
#define OBSERVER_SCOPE(name)
#define OBSERVER_SCOPE_BEGIN(name)
#define OBSERVER_SCOPE_END(name)
#define OBSERVER_DECLARE_THREAD(name)
#define OBSERVER_INIT_INSTRUMENTATION()
#define OBSERVER_STOP_INSTRUMENTATION()
#endif