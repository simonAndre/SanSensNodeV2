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
#include <ArduinoJson.h>

#include "SanSensNodeV2/platform_logger.h"
#include "SanSensNodeV2/Namespace.hpp"
#include "SanSensNodeV2/sandatacollector/SanDataCollector.hpp"
#include "SanSensNodeV2/sansensnode/SanCodedStr.hpp"
#include "SanSensNodeV2/sansensnode/SanSensNodeV2.hpp"

#if SANSENSNODE_EMBEDDED_MODE
#include <Arduino.h>
#endif

namespace sanSensNode
{
    // using SANSENSNODE_NAMESPACE::DeepSleep;
    using SANSENSNODE_NAMESPACE::SanCodedStr;
    using SANSENSNODE_NAMESPACE::SanDataCollector;
    using SANSENSNODE_NAMESPACE::SanSensNodeV2;
} // namespace sanSensNode
