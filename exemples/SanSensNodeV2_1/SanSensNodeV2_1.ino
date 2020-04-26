#include <SanSensNodeV2.h>
// #include <Arduino.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22 // DHT 22  (AM2302)
#define LED1PIN 16
#define LED2PIN 17
#define G_DURATION 10
#define P_FACTOR 1
#define DHT_WAITTIMEMS 2000
#define SANSENSNODE_SKETCHVERSION 011
bool collectdatadht(SanDataCollector collector);
void setupdevice();

DHT dht(DHTPIN, DHTTYPE, 30);
SanSensNodeV2 *_sensorNode;

int _dhtWarmupTime = DHT_WAITTIMEMS;

bool dht_setup()
{
    Serial.println("dht begin");
    // delay(1000);
    dht.begin();
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
}
void loop()
{
    _sensorNode->Loop();
}

bool collectdatadht(SanDataCollector *collector)
{
    _sensorNode->waitListeningIOevents(_dhtWarmupTime);
    //read humidity
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
        return false;
    }

    Serial.print("DHT data t=");
    Serial.print(t);
    Serial.print("°c ,H=");
    Serial.print(h);
    Serial.println("%");
    if (collector)
    {
        collector->Add_i("test_int", 999);
        collector->Add_f("temp", t);
        collector->Add_f("humi", h);
    }
    return true;
}

bool InputMessageAction(SanCodedStrings data)
{
    bool ledon;
    if (data.TryParseValue_b("led", &ledon))
    {
        Serial.print("led:");
        Serial.println(ledon);
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