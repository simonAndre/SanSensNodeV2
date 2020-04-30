#include <SanSensNodeV2.h>
// #include <Arduino.h>
#include <DHTesp.h>
#define DHTPIN 4
#define LED1PIN 16
#define LED2PIN 17
#define G_DURATION 10
#define P_FACTOR 1
#define DHT_WAITTIMEMS 2000
#define SANSENSNODE_SKETCHVERSION 020
bool collectdatadht(SanDataCollector collector);
void setupdevice();

DHTesp dht;
SanSensNodeV2 *_sensorNode;

int _dhtWarmupTime = DHT_WAITTIMEMS;

bool dht_setup()
{
    logdebugLn("dht begin");
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
    logdebugLn("setupdevice done");
}

void setup()
{
    pinMode(LED1PIN, OUTPUT); // Initialize the LED2PIN pin as an output
    pinMode(LED2PIN, OUTPUT); // Initialize the LED2PIN pin as an output

    _sensorNode = new SanSensNodeV2("atp1", "serenandre", "moustik77", "192.168.2.151", false, G_DURATION, P_FACTOR);
    _sensorNode->SetSetupDeviceCallback(setupdevice);
    _sensorNode->SetCollectDataCallback(collectdatadht);
    SanSensNodeV2::SetInputMessageCallback(InputMessageAction);
    _sensorNode->Setup();
    logdebugLn("sketch setup done");
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

    loginfoLn("DHT data t=%f°c ,H=%f%", th.temperature, th.humidity);
    if (collector)
    {
         collector->Add_f("temp", th.temperature);
        collector->Add_f("humi", th.humidity);
    }
    return true;
}

bool InputMessageAction(SanCodedStrings data)
{
    bool ledon;
    if (data.TryParseValue_b("led", &ledon))
    {
        logdebugLn("led:%i", ledon);
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