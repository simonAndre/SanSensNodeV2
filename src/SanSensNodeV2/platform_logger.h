#pragma once
#include "Configuration.hpp"
#if LOG_EN_DEFAULT == true
#include "../SanArduinoLogger/SanArduinoLogger.h"			// pour arduino : link to the Serial putchar
#include "../SanArduinoLogger/SanBufferLogger.h"
using PlatformLogger = PlatformLogger_t<SanBufferLogger<1024>>;


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

namespace SANSENSNODE_NAMESPACE
{
inline static void SetLogTimeStart(){
    SanBufferLogger<1024>::SetLogTimeStart();
    // _timeAtLoopStart = millis();
}
}