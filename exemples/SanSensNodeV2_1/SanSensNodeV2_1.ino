
// #define ARDUINO 150 //test sAN doit être setté par l'IDE
#include <DHTesp.h> //use https://github.com/beegee-tokyo/DHTesp
#include <SanSensNodeV2.h>
// #include <Arduino.h>
#define DHTPIN 4
#define LED1PIN 16
#define LED2PIN 17
#define SANSENSNODE_TOUCHPADGPIO 13 // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)
#define G_DURATION 60
#define P_FACTOR 1
#define DHT_WAITTIMEMS 0
#define SANSENSNODE_SKETCHVERSION 020
bool collectdatadht(SanDataCollector collector);
void setupdevice();

DHTesp dht;
SanSensNodeV2 *_sensorNode;

int _dhtWarmupTime = DHT_WAITTIMEMS;

bool dht_setup()
{
    logdebug("dht begin\n");
    dht.setup(DHTPIN, DHTesp::DHT_MODEL_t::DHT22);
    // delay(1000);
    return true;
}
bool led1On()
{
    digitalWrite(LED1PIN, HIGH);
    _sensorNode->waitListeningIOevents(250);
    digitalWrite(LED1PIN, LOW);
    return false;
}
bool led2On()
{
    digitalWrite(LED2PIN, HIGH);
    _sensorNode->waitListeningIOevents(250);
    digitalWrite(LED2PIN, LOW);
    return false;
}
void setupdevice()
{
    //hook up additional menu entries
    SubMenu *device_menu = _sensorNode->getDeviceMenu();
    device_menu->addMenuitemUpdater("DHT warmup time (ms)", &_dhtWarmupTime);
    device_menu->addMenuitemCallback("DHT reset", dht_setup);
    device_menu->addMenuitemCallback("tilt led 1", led1On);
    device_menu->addMenuitemCallback("tilt led 2", led2On);

    dht_setup();
    logdebug("setupdevice done\n");
}

void setup()
{
    pinMode(LED1PIN, OUTPUT); // Initialize the LED2PIN pin as an output
    pinMode(LED2PIN, OUTPUT); // Initialize the LED2PIN pin as an output

    _sensorNode = new SanSensNodeV2("atp1", "serenandre", "moustik77", "192.168.2.151",  G_DURATION, P_FACTOR);
    _sensorNode->SetSetupDeviceCallback(setupdevice);
    _sensorNode->SetCollectDataCallback(collectdatadht);
    SanSensNodeV2::SetInputMessageCallback(InputMessageAction);
    _sensorNode->Setup();
    logdebug("sketch setup done\n");
}
void loop()
{
    _sensorNode->Loop();
}

bool collectdatadht(SanDataCollector *collector)
{
    _sensorNode->waitListeningIOevents(_dhtWarmupTime);
    TempAndHumidity th = dht.getTempAndHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(th.temperature) || isnan(th.humidity))
    {
        logerror("Failed to read from DHT sensor!\n");
        return false;
    }

    loginfo("DHT data t=%f°c ,H=%f\n", th.temperature, th.humidity);
    if (collector)
    {
         collector->Add("temp", th.temperature);
        collector->Add("humi", th.humidity);
    }
    return true;
}

bool InputMessageAction(SanCodedStrings data)
{
    bool ledon;
    if (data.TryParseValue_b("led", &ledon))
    {
        logdebug("led:%i\n", ledon);
        if (ledon)
        {
            digitalWrite(LED2PIN, HIGH);
            _sensorNode->waitListeningIOevents(500);
            digitalWrite(LED2PIN, LOW);
        }
        else
            digitalWrite(LED2PIN, LOW);
    }
    return true;
}