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

bool collectdatadht(SanDataCollector collector);
void setupdevice();

DHT dht(DHTPIN, DHTTYPE, 30);
SanSensNodeV2 _sensorNode("atp1", "serenandre", "moustik77", "192.168.2.151", true, G_DURATION, P_FACTOR);

void setupdevice()
{
    Serial.println("dht begin");
    dht.begin(DHTPIN);
}

void setup()
{
    pinMode(LED2PIN, OUTPUT); // Initialize the LED2PIN pin as an output
    _sensorNode.SetSetupDeviceCallback(setupdevice);
    _sensorNode.SetCollectDataCallback(collectdatadht);
    SanSensNodeV2::SetInputMessageCallback(InputMessageAction);
    _sensorNode.Setup();
}
void loop()
{
    _sensorNode.Loop();
}

bool collectdatadht(SanDataCollector collector)
{
    _sensorNode.waitListeningIOevents(DHT_WAITTIMEMS);
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

    collector.Add_i("test_int", 999);
    collector.Add_f("temp", t);
    collector.Add_f("humi", h);

    Serial.print("DHT data t=");
    Serial.print(t);
    Serial.print("°c ,H=");
    Serial.print(h);
    Serial.println("%");
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
            _sensorNode.waitListeningIOevents(500);
            digitalWrite(LED2PIN, LOW);
        }
        else
            digitalWrite(LED2PIN, LOW);
    }
    return true;
}