#pragma once
#include <DHTesp.h> //use https://github.com/beegee-tokyo/DHTesp

#define DHT_WAITTIMEMS 0

class DHT22 : public devicePlugin
{
private:
    uint8_t _dhtpin;
    DHTesp dht;
    int _dhtWarmupTime = DHT_WAITTIMEMS;

public:
    DHT22(uint8_t dhtpin) : devicePlugin();
    ~DHT22();
    virtual bool collectdata(JsonColl *collector) override;
    virtual void setupdevice(SubMenu &device_menu) override;
    virtual void onInputMessage(SanCodedStr data) override;
    bool dht_setup();
};

DHT22::DHT22(uint8_t dhtpin)
{
    this->_dhtpin = dhtpin;
}

DHT22::~DHT22()
{
}

void DHT22::setupdevice(SubMenu &device_menu)
{
    //hook up additional menu entries
    // SubMenu *device_menu = _sensorNode->getDeviceMenu();
    device_menu->addMenuitemUpdater("DHT warmup time (ms)", &_dhtWarmupTime);
    device_menu->addMenuitemCallback("DHT reset", dht_setup);
    dht_setup();
}
bool DHT22::collectdata(JsonColl *collector)
{
    // _sensorNode->waitListeningIOevents(_dhtWarmupTime);
    delay(_dhtWarmupTime);
    
    TempAndHumidity th = dht.getTempAndHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(th.temperature) || isnan(th.humidity))
    {
        logerror("Failed to read from DHT sensor!\n");
        return false;
    }
    double voltage = ReadVoltage();
    loginfo("DHT data t=%f°c ,H=%f, Vpower=%f\n", th.temperature, th.humidity, voltage);

    if (collector)
    {
        collector->add("temp", th.temperature);
        collector->add("humi", th.humidity);
        collector->add("V", voltage);
    }
    return true;
}
void DHT22::onInputMessage(SanCodedStr data)
{
}
bool DHT22::dht_setup()
{
    dht.setup(_dhtpin, DHTesp::DHT_MODEL_t::DHT22);
    return true;
}