#pragma once

#define LOG_EN_DEFAULT true // set false to disable all log and lower the program size from the logging library.
#define LOG_LEVEL LOG_LEVEL_DEBUG

#define SANSENSNODE_TOUCHPADGPIO 15 // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)
#define SANSENSNODE_MQTT_NODENAME "mynode" // device node name, to be overrided in the NetworkSettings.h secret file
#define SANSENSNODE_WIFI_SSID "todefine"   //id of wifi network, to be overrided in the NetworkSettings.h secret file
#define SANSENSNODE_WIFI_PASSWD "todefine" // wifi password, to be overrided in the NetworkSettings.h secret file
#define SANSENSNODE_DEVICE_GDURATION 60    //measurement cycle duration ie: sleep time (in seconds) (in awake mode and sleep mode)
#define SANSENSNODE_DEVICE_PFACTOR 1       //publication and mqtt connection frequency (in G multiples)
#define SANSENSNODE_TOUCHPADGPIO 15        // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)

#define SANSENSNODE_MAX_MEASURES_ATTEMPTS 3
#define SANSENSNODE_WIFITRIALSINIT 5
#define SANSENSNODE_WIFIWAITTIMEMS 150
#define SANSENSNODE_MQTT_WAITFORMQTTSENDLOOP 5
#define SANSENSNODE_MQTT_TOPICBASENAME "/sansensnet/" //[build topic part] : base path for the mqtt topic
#define SANSENSNODE_MQTT_APPENDNODENAME // [build topic part] : to append the nodename to SANSENSNODE_MQTT_TOPICBASENAME
#define SANSENSNODE_MQTT_INTOPICSUFFIX "/in" // [build topic part] : suffix for the in-topic  (command messages to send orders the device)
#define SANSENSNODE_MQTT_OUTTOPICSUFFIX "/out" // [build topic part] : suffix for the out-topic (data messages flowing out of the device)
#define SANSENSNODE_MQTT_WAITFORMQTTRECEIVELOOP 0       // number of waiting loop to let the mqtt server send the in-messages (0 can be fine)
#define SANSENSNODE_MQTT_SERVER "192.168.1.254"         // mqtt server, to be overrided in the NetworkSettings.h secret file
#define SANSENSNODE_MQTT_PORT 1883
#define SANSENSNODE_MQTT_RECEIVEMESSAGE_SIZE 50
#define SANSENSNODE_MQTT_SUBSCRIBEATSTART true
#define SANSENSNODE_MQTT_ATTEMPTSNB 3
#undef MQTT_MAX_PACKET_SIZE      // un-define max packet size
#define MQTT_MAX_PACKET_SIZE 200 // fix for MQTT client dropping messages over 128B
#define SANSENSNODE_STARTINGCPUFREQ 80
#define SANSENSNODE_STRING_BUFFER_SIZE 32
#define SANSENSNODE_STARTSAWAKEN false      // 1=start in awake mode for the first boot
#define SANSENSNODE_WAITFORSERIALDELAY 400 // on setup : delay to let serial respond
#define SANSENSNODE_WAITLOOPDELAYMS 100 // define the atomic wait time in ms between 2 serial checks, see sanSensNet::waitWithSerialCheck method
#define SANSENSNODE_TOUCHPADTHRESHOLD 40 // touchepad wakup sensibility (greater is more sensitive)
#define SANSENSNODE_FIRSTBOOTDELAYWAITINGMENU 5       // in seconds, delay to wait for a user interraction to display menu after the firt boot or an awake by the touchpad
#define SANSENSNODE_SENSORS_EXP_SENSORSARRSIZE 5      // size of the sensors array
#define SANSENSNODE_NOTIMPL "method not implemented"

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
