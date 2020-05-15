#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SanSensNodeV2.h>
#define DS18B20_WAITTIMEMS 2000
namespace SANSENSNODE_NAMESPACE
{
    class DS18B20 : public SensorPlugin
    {
    private:
        uint8_t _oneWireBus;
        // Number of temperature devices found
        int numberOfDevices;
        // We'll use this variable to store a found device address
        DeviceAddress tempDeviceAddress;
        DallasTemperature *sensors;
        void waitWarmup();

    public:
        DS18B20(uint8_t oneWireBus);
        virtual ~DS18B20();
        virtual bool collectdata(JsonColl &collector) override;
        virtual void firstSetup() override;
        virtual void setupsensor() override;
        virtual void setMenu(SubMenu &sensor_menu);
        virtual void onInputMessage(flyingCollection::SanCodedStr &data);
    };
}