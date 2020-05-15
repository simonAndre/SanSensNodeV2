#define SANSENSNODE_SKETCHVERSION 0343
#define ARDUINO 150 //test sAN doit être setté par l'IDE

#define USE_DS18B20
#define USE_DHT22
#define USE_TEST
#define USE_VOLTAGEPROBE

#include <SanSensNodeV2.h>
#include "NetworkSettings.h" // this optional file contains overrided settings & macro (not pushed on the remote repository)

const uint8_t DHTpin = 4;
const uint8_t oneWireBus = 5;


#define SANSENSNODE_TOUCHPADGPIO 15 // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)
#define SANSENSNODE_DEVICE_GDURATION 10
#define SANSENSNODE_DEVICE_PFACTOR 1

//#define SANSENSNODE_LOG_USEBUFFER // if defined : use a buffer, you need to call logflush() to flush the log buffer the logger is available even before the activation of Serial.begin \
                                  // if not defined : immediate logging


SanSensNodeV2 *_sensorNode;

void setup()
{
    _sensorNode = new SanSensNodeV2(SANSENSNODE_MQTT_NODENAME, SANSENSNODE_WIFI_SSID, SANSENSNODE_WIFI_PASSWD, SANSENSNODE_MQTT_SERVER, SANSENSNODE_MQTT_TOPICBASENAME, SANSENSNODE_DEVICE_GDURATION, SANSENSNODE_DEVICE_PFACTOR);
    if (_sensorNode->isFirstInit())
    {
    }

    DHT22 *dht = new DHT22(DHTpin);
    _sensorNode->addDevice(dht);

    TestPlugin *testplugin = new TestPlugin();
    _sensorNode->addDevice(testplugin);

    VoltageProbe *voltage = new VoltageProbe();
    _sensorNode->addDevice(voltage);

    DS18B20 *ds18b20 = new DS18B20(oneWireBus);
    _sensorNode->addDevice(ds18b20);

    _sensorNode->Setup();

    logdebug("sketch setup done\n");
}
void loop()
{
    _sensorNode->Loop();
}
