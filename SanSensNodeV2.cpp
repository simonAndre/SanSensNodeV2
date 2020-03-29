
#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "Arduino.h"
#include "SanSensNodeV2.h"
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>

#define MAX_MEASURES_ATTEMPTS 7
#define G_INITIAL 5 /* default measurement cycle time ie: sleep time (in seconds) */
#define P_INITIAL 1 /* default publication frequency (in G multiples) */
#define WIFITRIALSINIT 10
#define WAITFORMQTTLOOP 10
#define MQTTPORT 1883

RTC_DATA_ATTR int _bootCount = 0;                     // store "boot" (restart after sleep) counting (in the RTC resilient RAM memory)
RTC_DATA_ATTR bool _awakemode = false;                // while in awake mode => don't go to sleep on each G cycle until a sleep order has been received
RTC_DATA_ATTR int _G_seconds = G_INITIAL;             // measurement cycle time ie: sleep time (in seconds) (in awake mode and sleep mode)
RTC_DATA_ATTR int _Pfactor = P_INITIAL;               // publication and mqtt connection frequency (in G multiples)
RTC_DATA_ATTR bool _serial = true;                    // false to disable output serial log
RTC_DATA_ATTR bool _lowEnergyMode = true;             // false to disable output serial log
RTC_DATA_ATTR uint8_t _wifitrialsmax;                 // nb max of attemps to check the wifi network for a wakeup cycle
RTC_DATA_ATTR uint8_t _waitforMqtt = WAITFORMQTTLOOP; // nb of loop to wait for mqtt server answer
RTC_DATA_ATTR int _GcycleIx = 0;                      // store the current cycle index (G or measurement cycle)
RTC_DATA_ATTR int _EXPwifiwait = 1000;                // EXPERIMENTAL : temps d'attente suite allmuage wifi pour que le MQTT puisse répondre
RTC_DATA_ATTR int _EXPmqttattemps = 3;                // EXPERIMENTAL : nb de tentatives pour connexion au server mqtt
bool _detailedFrame = false;

WiFiClient espClient;
PubSubClient client(espClient);

SanSensNodeV2::SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, bool lowEnergyMode, int G, int Pfactor)
{
    if (_bootCount == 0) // première initialisation, ensuite on rentre dans ce constructeur à chaque reveil donc on ne doit pas réinitialiser les variables stockées dnas la RAM de la RTC
    {
        SanSensNodeV2::_nodename = (char *)nodename;
        SanSensNodeV2::_ssid = (char *)ssid;
        SanSensNodeV2::_password = (char *)wifipasswd;
        SanSensNodeV2::_mqtt_server = (char *)mqttserver;
        SanSensNodeV2::_maxMeasurementAttenmpts = MAX_MEASURES_ATTEMPTS;
        SanSensNodeV2::_measurementAttenmpts = 0;
        _G_seconds = G;
        _Pfactor = Pfactor;
        _lowEnergyMode = lowEnergyMode;
        _wifitrialsmax = WIFITRIALSINIT;
        if (lowEnergyMode)
            _wifitrialsmax = _wifitrialsmax / 3;
    }
    ++_bootCount;
}
// SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, SETUPDEVICES_SIGNATURE, COLLECTDATA_SIGNATURE);
// {
//     SanSensNodeV2::_nodename = (char *)nodename;
//     SanSensNodeV2::_ssid = (char *)ssid;
//     SanSensNodeV2::_password = (char *)wifipasswd;
//     SanSensNodeV2::_mqtt_server = (char *)mqttserver;
//     SanSensNodeV2::_maxMeasurementAttenmpts = MAX_MEASURES_ATTEMPTS;
//     SanSensNodeV2::_measurementAttenmpts = 0;
//     SanSensNodeV2::callback_collectdata = callback_collectdata;
//     SanSensNodeV2::callback_setupdevices = callback_setupdevices;
// }

SanSensNodeV2::~SanSensNodeV2()
{
}
//setup the callback called to setup the sensors
void SanSensNodeV2::SetSetupDeviceCallback(SETUPDEVICES_SIGNATURE)
{
    SanSensNodeV2::callback_setupdevices = callback_setupdevices;
}
// called to collect sensors data to be sent via mqtt
void SanSensNodeV2::SetCollectDataCallback(COLLECTDATA_SIGNATURE)
{
    SanSensNodeV2::callback_collectdata = callback_collectdata;
}
void SanSensNodeV2::SetInputMessageCallback(INPUTMESSAGE_SIGNATURE)
{
    callback_inputmessage = _callback_inputmessage;
}

bool mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    String a;
    char *myvalue;
    bool asleep;

    SanCodedStrings pyldic((char *)payload);

    if (pyldic.TryParseValue_b("sl", &asleep))
    {
        _awakemode = !asleep;
        Serial.print("sleep:");
        Serial.println(_awakemode);
    }
    if (pyldic.TryParseValue_b("serial", &_serial))
    {
        Serial.print("serial:");
        Serial.println(_serial);
        if (_serial)
            Serial.begin(115200);
        else
            Serial.end();
    }
    if (pyldic.TryParseValue_i("G", &_G_seconds))
    {
        Serial.print("G(seconds):");
        Serial.println(_G_seconds);
    }
    if (pyldic.TryParseValue_i("P", &_Pfactor))
    {
        Serial.print("P(G multiple):");
        Serial.println(_Pfactor);
    }
    if (pyldic.TryParseValue_i("wfmqtt", &_waitforMqtt))
    {
        Serial.print("WaitforMqtt:");
        Serial.println(_waitforMqtt);
    }
    pyldic.TryParseValue_b("details", &_detailedFrame);
    pyldic.TryParseValue_i("wifiwait", &_EXPwifiwait);
    pyldic.TryParseValue_i("wifiattemps", &_EXPmqttattemps);
    if (callback_inputmessage != NULL)
        return callback_inputmessage(pyldic);

    return true;
}

void SanSensNodeV2::Setup_wifi()
{

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SanSensNodeV2::_ssid);

    WiFi.begin(SanSensNodeV2::_ssid, SanSensNodeV2::_password);
    int nb = 0;
    while (WiFi.status() != WL_CONNECTED && nb++ < _wifitrialsmax)
    {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        SanSensNodeV2::DeepSleep();
    }
    // randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(SanSensNodeV2::_mqtt_server, MQTTPORT);
    client.setCallback(mqttCallback);
    delay(_EXPwifiwait);
}

bool SanSensNodeV2::mqttSubscribe()
{
    if (!client.connected())
    {
        Serial.println("can't subscribe: no mqtt connection");
        return false;
    }
    std::string intopic(SanSensNodeV2::_mqttTopicBaseName);
    intopic.append(_nodename);
    intopic.append("/in");
    if (!client.subscribe(intopic.c_str(), 1))
    {
        Serial.print("subscription to topic ");
        Serial.print(intopic.c_str());
        Serial.println(" failed!");
        return false;
    }
    Serial.println("mqtt subscription OK");
    return true;
}
bool SanSensNodeV2::mqttConnect()
{
    uint8_t i = 0;
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("MQTT connection: ");
        // Attempt to connect
        if (!client.connect(_nodename))
        {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            if (++i >= _EXPmqttattemps)
                return false;
        }
        Serial.println("OK");
    }
    return true;
}

bool SanSensNodeV2::WaitforMqtt(int nb)
{
    int l;
    // if (_awakemode)
    //   nb = 1;
    for (int l = 0; l <= nb; l++)
    {
        if (!client.loop()) //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
            return false;   // the client is no longer connected
        delay(20);
    }
}
bool SanSensNodeV2::collectMeasurement_internal(SanDataCollector dc)
{
    //collect data and fill _datacollector with it
    dc.Add_s("device", (const char *)_nodename);
    dc.Add_i("boot", _bootCount);
    double timeSinceStartup = esp_timer_get_time() / 1000000;
    dc.Add_d("uptime", timeSinceStartup);
    dc.Add_i("measurements", _measurementAttenmpts);
    if (_detailedFrame)
    {
        dc.Add_i("G", _G_seconds);
        dc.Add_i("P", _Pfactor);
        dc.Add_i("freeram", (int)heap_caps_get_free_size(MALLOC_CAP_8BIT));
        dc.Add_i("Gcylcle", _GcycleIx);
        dc.Add_b("LowEnergyMode", _lowEnergyMode);
        dc.Add_b("Serial", _serial);
        dc.Add_i("wifitrialsmax", _wifitrialsmax);
        dc.Add_i("waitforMqtt", _waitforMqtt);
        dc.Add_i("mqttattemps", _EXPmqttattemps);
        dc.Add_i("wifiwait", _EXPwifiwait);
    }
    return true;
}

bool SanSensNodeV2::collectMeasurement(SanDataCollector dc)
{
    if (!collectMeasurement_internal(dc))
        return false;
    if (callback_collectdata != NULL)
    {
        if (!callback_collectdata(dc))
            return false;
    }
    return true;
}

bool SanSensNodeV2::mqttpubsub(SanDataCollector dc)
{
    Setup_wifi();
    if (!client.connected())
        if (!mqttConnect())
            return false;

    mqttSubscribe(); // todo : action en cas de défaut de souscription??

    // WaitforMqtt(15); // to wait for incoming messages

    if (_measurementAttenmpts >= _maxMeasurementAttenmpts)
        dc.Add_b("SensorIssue", true);

    char buffer[dc.getbufferSize()];
    size_t size = dc.Serialize(buffer);

    std::string outopic(SanSensNodeV2::_mqttTopicBaseName);
    outopic.append(_nodename);
    outopic.append("/out");
    Serial.print("publish on ");
    Serial.print(outopic.c_str());
    Serial.print(", taille message: ");
    Serial.println(size);
    client.publish(outopic.c_str(), buffer, true);
    Serial.print("message: ");
    Serial.println(buffer);
    Serial.println("publication mqtt OK");

    return WaitforMqtt(_waitforMqtt);
    wifiOff();
}

void SanSensNodeV2::wifiOff()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void SanSensNodeV2::Setup()
{
    if (_lowEnergyMode)
        setCpuFrequencyMhz(80); // set the lowest frequency possible to save energy (we)
    // ++_bootCount;
    if (_serial)
    {
        delay(1000); // pour attendre port serie
        Serial.begin(115200);
        Serial.print("G duration is ");
        Serial.println(_G_seconds);
        Serial.print("P factor is ");
        Serial.println(_Pfactor);
    }
    // esp_sleep_enable_timer_wakeup(_delayloop * uS_TO_S_FACTOR);

    if (callback_setupdevices != NULL)
        callback_setupdevices();
}

void SanSensNodeV2::Loop()
{
    _GcycleIx++;
    Serial.print("G index:");
    Serial.println(_GcycleIx);

    _measurementAttenmpts = 1;

    SanDataCollector datacollector;
    Serial.println("measure");
    while (!SanSensNodeV2::collectMeasurement(datacollector) && _measurementAttenmpts <= _maxMeasurementAttenmpts)
    {
        _measurementAttenmpts++;
    }
    Serial.println("measure end");

    if (_GcycleIx % _Pfactor == 0)
        SanSensNodeV2::mqttpubsub(datacollector);

    if (!_awakemode)
    {
        Serial.print("Sleep for s");
        Serial.println(_G_seconds);
        Serial.flush();
        SanSensNodeV2::DeepSleep();
    }
    Serial.print("Wait for s");
    Serial.println(_G_seconds);
    delay(1000 * _G_seconds);
}

void SanSensNodeV2::DeepSleep()
{
    Serial.println("Going to sleep...");
    // WiFi.disconnect(true);
    // WiFi.mode(WIFI_OFF);
    btStop();

    adc_power_off();
    esp_wifi_stop();
    esp_bt_controller_disable();

    // Configure the timer to wake up
    esp_sleep_enable_timer_wakeup(_G_seconds * 1000000L);

    // Go to sleep
    esp_deep_sleep_start();
}