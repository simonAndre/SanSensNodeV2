#pragma once

#include "SanSensNodeV2/Configuration.hpp"

#if !SANSENSNODE_DEBUG
#ifdef __clang__
#pragma clang system_header
#elif defined __GNUC__
#pragma GCC system_header
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <map>
#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include <consoleMenu.h>
#include <flyingCollection.h>

#undef MQTT_MAX_PACKET_SIZE      // un-define max packet size
#define MQTT_MAX_PACKET_SIZE 250 // fix for MQTT client dropping messages over 128B


#include "SanSensNodeV2/platform_logger.h"
#include "SanSensNodeV2/Namespace.hpp"
#include "SanSensNodeV2/sansensnode/SanSensNodeV2.hpp"

#if SANSENSNODE_EMBEDDED_MODE
#include <Arduino.h>
#endif

namespace sanSensNode
{
    using SANSENSNODE_NAMESPACE::JsonColl;
    using SANSENSNODE_NAMESPACE::SanSensNodeV2;
} // namespace sanSensNode
