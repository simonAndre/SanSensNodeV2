#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>


class DS18B20 : public devicePlugin
{
private:
    uint8_t _oneWireBus;
    // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
    OneWire oneWire(_oneWireBus);
    // Pass our oneWire reference to Dallas Temperature.
    DallasTemperature sensors(&oneWire);
    // Number of temperature devices found
    int numberOfDevices;
    // We'll use this variable to store a found device address
    DeviceAddress tempDeviceAddress;
    void DS18B20::printAddress(DeviceAddress deviceAddress);

public:
    DS18B20(uint8_t oneWireBus) : devicePlugin();
    ~DS18B20();
    virtual bool collectdata(JsonColl *collector) override;
    virtual void setupdevice(SubMenu &device_menu) override;
    virtual void onInputMessage(SanCodedStr data) override;
};

DS18B20::pluginOneWire(uint8_t oneWireBus){
    this->_oneWireBus = oneWireBus;
}

DS18B20::~pluginOneWire()
{
}

void DS18B20::setupdevice(SubMenu &device_menu)
{
    sensors.begin();

    // Grab a count of devices on the wire
    numberOfDevices = sensors.getDeviceCount();

    // locate devices on the bus
    Serial.print("Locating devices...");
    Serial.print("Found ");
    Serial.print(numberOfDevices, DEC);
    Serial.println(" devices.");

    // Loop through each device, print out address
    for (int i = 0; i < numberOfDevices; i++)
    {
        // Search the wire for address
        if (sensors.getAddress(tempDeviceAddress, i))
        {
            Serial.print("Found device ");
            Serial.print(i, DEC);
            Serial.print(" with address: ");
            printAddress(tempDeviceAddress);
            Serial.println();
        }
        else
        {
            Serial.print("Found ghost device at ");
            Serial.print(i, DEC);
            Serial.print(" but could not detect address. Check power and cabling");
        }
    }
}

bool DS18B20::collectdata(JsonColl *collector)
{
    sensors.requestTemperatures(); // Send the command to get temperatures

    float T2 = sensors.getTempCByIndex(0);
    float T3 = sensors.getTempCByIndex(1);
    loginfo("one wire temp1=%f°c, temp2=%f°c\n", T2, T3);

   
    // Loop through each device, print out temperature data
    for (int i = 0; i < numberOfDevices; i++)
    {
        // Search the wire for address
        if (sensors.getAddress(tempDeviceAddress, i))
        {
            // Output the device ID
            Serial.print("Temperature for device: ");
            Serial.println(i, DEC);
            // Print the data
            float tempC = sensors.getTempC(tempDeviceAddress);
            Serial.print("Temp C: ");
            Serial.print(tempC);
            char buf[3];
            snprintf(buf,3, "T%i", i);
            if (collector)
            {
                collector->add(buf, tempC);
            }
        }
    }
}
void DS18B20::onInputMessage(SanCodedStr data)
{
}

// function to print a device address
void DS18B20::printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
            Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}
