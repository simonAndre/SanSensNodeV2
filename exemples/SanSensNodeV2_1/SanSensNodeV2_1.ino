#define ARDUINO 150 //test sAN doit être setté par l'IDE

#define USE_DS18B20
#define USE_DHT22
#define USE_TEST
#define USE_VOLTAGEPROBE

#include <SanSensNodeV2.h>

const uint8_t DHTpin = 4;
const uint8_t oneWireBus = 5;
#define SANSENSNODE_TOUCHPADGPIO 15 // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)
#define G_DURATION 10
#define P_FACTOR 1
#define SANSENSNODE_SKETCHVERSION 0341

#define LOG_ECHO_EN_DEFAULT 1

#undef MQTT_MAX_PACKET_SIZE      // un-define max packet size
#define MQTT_MAX_PACKET_SIZE 250 // fix for MQTT client dropping messages over 128B

//callback to implement if needed
bool collectdata(JsonColl &collector);
// void InputMessageAction(flyingCollection::SanCodedStr const &data);
// void setupdevice(SubMenu &device_menu);

SanSensNodeV2 *_sensorNode;

void setup()
{

    _sensorNode = new SanSensNodeV2("atp1", "serenandre", "moustik77", "192.168.2.151", G_DURATION, P_FACTOR);
    if (_sensorNode->isFirstInit())
    {
        //todo setter les paramètres wifi
    }
    // _sensorNode->SetSetupDeviceCallback(setupdevice);       //optional
    // _sensorNode->SetCollectDataCallback(collectdata);       //optional

    logdebug("add DHT22\n");
    DHT22 *dht = new DHT22(DHTpin);
    _sensorNode->addDevice(dht);

    logdebug("add tetplugin\n");
    TestPlugin *testplugin = new TestPlugin();
    _sensorNode->addDevice(testplugin);

    logdebug("add VoltageProbe\n");
    VoltageProbe *voltage = new VoltageProbe();
    _sensorNode->addDevice(voltage);

    logdebug("add DS18B20\n");
    DS18B20 *ds18b20 = new DS18B20(oneWireBus);
    _sensorNode->addDevice(ds18b20);

    // SanSensNodeV2::SetInputMessageCallback(InputMessageAction);

    _sensorNode->Setup();

    logdebug("sketch setup done\n");
}
void loop()
{
    _sensorNode->Loop();
}
