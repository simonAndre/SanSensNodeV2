#pragma once
#include "Configuration.hpp"
#if LOG_EN_DEFAULT == true
#include <CircularBufferLogger.h>
using PlatformLogger = PlatformLogger_t<CircularLogBufferLogger<1024>>;

#if LOG_LEVEL >= LOG_LEVEL_INFO
#ifndef loginfoLn
#define loginfoLn(...)                 \
    PlatformLogger::info(__VA_ARGS__); \
    PlatformLogger::print("\n");
#endif
#else
#define loginfoLn(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#ifndef logdebugLn
#define logdebugLn(...)                \
    PlatformLogger::debug(__VA_ARGS__); \
    PlatformLogger::print("\n");
#endif
#else
#define logdebugLn(...)
#endif

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
#endif // LOG_EN_DEFAULT
