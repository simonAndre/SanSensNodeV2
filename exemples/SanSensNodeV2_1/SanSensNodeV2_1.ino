#include <DallasTemperature.h>



// #define ARDUINO 150 //test sAN doit être setté par l'IDE
#include <SanSensNodeV2.h>
#include "pluginOneWire.hpp"
#include "pluginDHT22.hpp"
// #include <Arduino.h>
const uint8_t DHTpin = 4;
const uint8_t oneWireBus = 5;
#define LED1PIN 16
#define LED2PIN 17
#define SANSENSNODE_TOUCHPADGPIO 15 // GPIO pin for touchpad wakeup (GPIO 4,0,2,15,13,12,14,27,33,32 only)
#define VOLTAGEPIN 33               //voltage measure (0 to 3.3v, use divided bridge)
#define ADC_a 0.00196164285990814   // voltage divided bridge : R1=990kH, R2=945kH
#define ADC_b 0.341586840689968
#define G_DURATION 10
#define P_FACTOR 1
#define SANSENSNODE_SKETCHVERSION 0341

#undef MQTT_MAX_PACKET_SIZE      // un-define max packet size
#define MQTT_MAX_PACKET_SIZE 250 // fix for MQTT client dropping messages over 128B

bool collectdatadht(JsonColl *collector);
void setupdevice();

SanSensNodeV2 *_sensorNode;


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
void setupdevice(SubMenu& device_menu)
{
    //hook up additional menu entries
    // SubMenu *device_menu = _sensorNode->getDeviceMenu();
    device_menu->addMenuitemCallback("tilt led 1", led1On);
    device_menu->addMenuitemCallback("tilt led 2", led2On);
    logdebug("setupdevice done\n");
}

void setup()
{
    pinMode(LED1PIN, OUTPUT); // Initialize the LED2PIN pin as an output
    pinMode(LED2PIN, OUTPUT); // Initialize the LED2PIN pin as an output

    _sensorNode = new SanSensNodeV2("atp1", "serenandre", "moustik77", "192.168.2.151", G_DURATION, P_FACTOR);
    _sensorNode->SetSetupDeviceCallback(setupdevice);
    _sensorNode->SetCollectDataCallback(collectdatadht);
    SanSensNodeV2::SetInputMessageCallback(InputMessageAction);

    DS18B20 *ds18b20 = new DS18B20(oneWireBus);
    _sensorNode->addDevice(ds18b20);
    DHT22 *dht = new DHT22(DHTpin);
    _sensorNode->addDevice(dht);
    _sensorNode->Setup();

    logdebug("sketch setup done\n");
}
void loop()
{
    _sensorNode->Loop();
}

bool collectdatadht(JsonColl *collector)
{
 
}

void InputMessageAction(SanCodedStr data)
{
    bool ledon;
    uint8_t ledpin{0};

    if (data.tryGetValue("led2", ledon))
    {
        ledpin = LED2PIN;
    }
    if (data.tryGetValue("led1", ledon))
    {
        ledpin = LED1PIN;
    }
    if(ledpin>0){
        logdebug("led-%i:%i\n",ledpin, ledon);
        if (ledon)
        {
            digitalWrite(ledpin, HIGH);
            // _sensorNode->waitListeningIOevents(500);
            // digitalWrite(ledpin, LOW);
        }
        else
            digitalWrite(ledpin, LOW);
    }
}

const uint8_t nb_readings = 20;

double ReadVoltage()
{
    //on fait une moyenne des lectures
    double reading = 0;
    for (size_t i = 0; i < nb_readings; i++)
    {
        reading += analogRead(VOLTAGEPIN);
        delay(10);
    }
    reading = reading / nb_readings;
    // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
    if (reading < 1 || reading > 4095)
        return 0;
    return ADC_a * reading + ADC_b;
}

double ReadVoltageOn3_3()
{
    double reading = analogRead(VOLTAGEPIN); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
    if (reading < 1 || reading > 4095)
        return 0;
    return -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
}