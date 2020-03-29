#include "SanDataCollector.h"
#include "SanCodedStrings.h"

#define COLLECTDATA_SIGNATURE bool (*callback_collectdata)(SanDataCollector)
#define SETUPDEVICES_SIGNATURE void (*callback_setupdevices)(void)
#define INPUTMESSAGE_SIGNATURE bool (*_callback_inputmessage)(SanCodedStrings)
static bool (*callback_inputmessage)(SanCodedStrings);

class SanSensNodeV2
{
private:
    char *_nodename, *_ssid, *_password, *_mqtt_server;
    uint8_t _maxMeasurementAttenmpts, _measurementAttenmpts;
    COLLECTDATA_SIGNATURE;
    SETUPDEVICES_SIGNATURE;

    bool mqttConnect();
    //allow the client to process incoming messages and maintain its connection to the server.
    bool WaitforMqtt(int nb);
    bool mqttSubscribe();
    bool collectMeasurement(SanDataCollector dc);
    // publish and subscribe to the respective MQTT chanels (open the connection and close after)
    bool mqttpubsub(SanDataCollector dc);
    void Setup_wifi();
    void wifiOff();
    bool collectMeasurement_internal(SanDataCollector dc);
    // bool mqttCallback(char *topic, uint8_t *payload, unsigned int length);
    const char *_mqttTopicBaseName = "/ssnet/";
    const char *_lostTopic = "/ssnet/lost"; // not implemented : when the sensor has not been initialized, it wait configuration data from this topic

public:
    // nodename : name for this node
    // ssid : id of wifi network
    // wifipasswd : password of wifi network
    // mqttserver : adress of mqtt server
    // lowEnergyMode : true to be as energy efficient as we can at the trade off of the power (runing at 80Mhz...)
    // G : measurement cycle duration ie: sleep time (in seconds) (in awake mode and sleep mode)
    // Pfactor : publication and mqtt connection frequency (in G multiples)
    SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, bool lowEnergyMode, int G, int Pfactor);
    // SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, SETUPDEVICES_SIGNATURE, COLLECTDATA_SIGNATURE);
    ~SanSensNodeV2();
    void Setup();
    void Loop();
    void SetCollectDataCallback(COLLECTDATA_SIGNATURE);
    void SetSetupDeviceCallback(SETUPDEVICES_SIGNATURE);
    void SetInputMessageCallback(INPUTMESSAGE_SIGNATURE);
    // go to deep sleep (with disabling wifi, bluetooth and so)
    void DeepSleep();
};
