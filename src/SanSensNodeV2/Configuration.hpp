#pragma once

#define LOG_EN_DEFAULT true // set false to disable all log and lower the program size from the logging library.
#define LOG_LEVEL LOG_LEVEL_INFO

#define SANSENSNODE_TOUCHPADGPIO 13 // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)

#define SANSENSNODE_MAX_MEASURES_ATTEMPTS 3
#define SANSENSNODE_WIFITRIALSINIT 5
#define SANSENSNODE_WIFIWAITTIMEMS 150
#define SANSENSNODE_WAITFORMQTTSENDLOOP 5
#define SANSENSNODE_WAITFORMQTTRECEIVELOOP 0
#define SANSENSNODE_MQTTPORT 1883
#define SANSENSNODE_MQTTATTEMPTSNB 3
#define MQTT_MAX_PACKET_SIZE 254
#define MQTT_MAX_TRANSFER_SIZE 150
#define SANSENSNODE_STARTINGCPUFREQ 80
#define SANSENSNODE_STRING_BUFFER_SIZE 32
#define SANSENSNODE_MQTTRECEIVEMESSAGE_SIZE 50
#define SANSENSNODE_MQTTSUBSCRIBEATSTART true
#define SANSENSNODE_STARTSAWAKEN false      // 1=start in awake mode for the first boot
#define SANSENSNODE_WAITFORSERIALDELAY 400 // on setup : delay to let serial respond
#define SANSENSNODE_WAITLOOPDELAYMS 100 // define the atomic wait time in ms between 2 serial checks, see sanSensNet::waitWithSerialCheck method
#define SANSENSNODE_TOUCHPADTHRESHOLD 40 // touchepad wakup sensibility (greater is more sensitive)

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
