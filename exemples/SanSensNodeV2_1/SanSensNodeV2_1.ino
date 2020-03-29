#include "SanSensNodeV2.h"
#include "DHT.h"
// #include "SanDataCollector.h"

#define DHTPIN 4
#define DHTTYPE DHT22 // DHT 22  (AM2302)
#define LED1PIN 16
#define LED2PIN 17
#define G_DURATION 10
#define P_FACTOR 1

bool collectdatadht(SanDataCollector collector);
void setupdevice();

DHT dht(DHTPIN, DHTTYPE, 30);
SanSensNodeV2 _sensorNode("atp1", "serenandre", "moustik77", "192.168.2.151", true, G_DURATION, P_FACTOR);

void setupdevice()
{
    dht.begin(DHTPIN);
}

void setup()
{
    pinMode(LED2PIN, OUTPUT); // Initialize the LED2PIN pin as an output
    _sensorNode.SetSetupDeviceCallback(setupdevice);
    _sensorNode.SetCollectDataCallback(collectdatadht);
    _sensorNode.SetInputMessageCallback(InputMessageAction);
    _sensorNode.Setup();
}
void loop()
{
    _sensorNode.Loop();
}

bool collectdatadht(SanDataCollector collector)
{
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
    Serial.println("collectdatadht start");
    collector.Add_s("test_str", "ma chaine");
    collector.Add_i("test_int", 999);
    collector.Add_f("temp", t);
    collector.Add_f("humi", h);
    Serial.println("collectdatadht fin");
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
            delay(500);
            digitalWrite(LED2PIN, LOW);
        }
        else
            digitalWrite(LED2PIN, LOW);
    }
    return true;
}