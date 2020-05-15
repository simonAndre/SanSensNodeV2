#pragma once
#ifdef __cplusplus

#include "SanSensNodeV2/Configuration.h"

#if !SANSENSNODE_DEBUG
#ifdef __clang__
#pragma clang system_header
#elif defined __GNUC__
#pragma GCC system_header
#endif
#endif
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdbool.h>
// #include <vector>
// #include <WiFiClient.h>
// #include <PubSubClient.h>
#include <EEPROM.h>
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
// #include <consoleMenu.h>
// #include <flyingCollection.h>

#undef MQTT_MAX_PACKET_SIZE      // un-define max packet size
#define MQTT_MAX_PACKET_SIZE 250 // fix for MQTT client dropping messages over 128B

#include "SanSensNodeV2/platform_logger.h"
#include "SanSensNodeV2/Namespace.h"

#include "SanSensNodeV2/sansensnode/SanSensNodeV2.h"

#ifdef USE_DS18B20
#include "SanSensNodeV2/plugins/DS18B20.h"
#endif
#ifdef USE_DHT22
#include "SanSensNodeV2/plugins/DHT22.h"
#endif
#ifdef USE_TEST
#include "SanSensNodeV2/plugins/TestPlugin.h"
#endif
#ifdef USE_VOLTAGEPROBE
#include "SanSensNodeV2/plugins/VoltageProbe.h"
#endif

namespace sanSensNode
{
    using SANSENSNODE_NAMESPACE::SensorPlugin;
    using SANSENSNODE_NAMESPACE::JsonColl;
    using SANSENSNODE_NAMESPACE::SanSensNodeV2;

#ifdef USE_DS18B20
    using SANSENSNODE_NAMESPACE::DS18B20;
#endif
#ifdef USE_DHT22
    using SANSENSNODE_NAMESPACE::DHT22;
#endif
#ifdef USE_TEST
    using SANSENSNODE_NAMESPACE::TestPlugin;
#endif
#ifdef USE_VOLTAGEPROBE
    using SANSENSNODE_NAMESPACE::VoltageProbe;
#endif

} // namespace sanSensNode

using namespace sanSensNode;

#else //ifdef __cplusplus

#error SanSensNode requires a C++ compiler, please change file extension to .cc or .cpp

#endif //ifdef __cplusplus
