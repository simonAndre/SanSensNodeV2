#pragma once

#define LOG_EN_DEFAULT true // set false to disable all log and lower the program size from the logging library.
#define LOG_LEVEL LOG_LEVEL_DEBUG 

#define SANSENSNODE_MAX_MEASURES_ATTEMPTS 7
#define SANSENSNODE_WIFITRIALSINIT 10
#define SANSENSNODE_WIFIWAITTIMEMS 1000
#define SANSENSNODE_WAITFORMQTTLOOP 10
#define SANSENSNODE_MQTTPORT 1883
#define SANSENSNODE_LOWENERGYFACTOR 3
#define SANSENSNODE_MQTTATTEMPTSNB 3
#define SANSENSNODE_STARTINGCPUFREQ 80
#define SANSENSNODE_STRING_BUFFER_SIZE 32
#define SANSENSNODE_STARTSAWAKEN 1      // 1=start in awake mode for the first boot
#define SANSENSNODE_WAITLOOPDELAYMS 100 // define the atomic wait time in ms between 2 serial checks, see sanSensNet::waitWithSerialCheck method

// computer or embedded?
#ifndef SANSENSNODE_EMBEDDED_MODE
#if defined(ARDUINO)                /* Arduino*/                 \
    || defined(__IAR_SYSTEMS_ICC__) /* IAR Embedded Workbench */ \
    || defined(__XC)                /* MPLAB XC compiler */      \
    || defined(__ARMCC_VERSION)     /* Keil ARM Compiler */      \
    || defined(__AVR)               /* Atmel AVR8/GNU C Compiler */
#define SANSENSNODE_EMBEDDED_MODE 1
#else
#define SANSENSNODE_EMBEDDED_MODE 0
#endif
#endif
