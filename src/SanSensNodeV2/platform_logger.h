#pragma once
#include "Configuration.h"
#if LOG_EN_DEFAULT == true
#include <LibPrintf.h>
#include "logger/SanBufferLogger.h"

using PlatformLogger = PlatformLogger_t<SanBufferLogger<1024>>;
inline static void ___setLogTimeStart()
{
    SanBufferLogger<1024>::SetLogTimeStart();
}
#define SetLogTimeStart() ___setLogTimeStart();

#else // LOG_EN_DEFAULT
#define logcritical(...)
#define logerror(...)
#define logwarning(...)
#define loginfo(...)
#define logdebug(...)
#define logflush()
#define loglevel(lvl)
#define logecho(echo)
#define logclear()
#define SetLogTimeStart()
#endif // LOG_EN_DEFAULT



