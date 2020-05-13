#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SanSensNodeV2.h>
namespace SANSENSNODE_NAMESPACE
{
    class DS18B20 : public DevicePlugin
    {
    private:
        uint8_t _oneWireBus;
        // Number of temperature devices found
        int numberOfDevices;
        // We'll use this variable to store a found device address
        DeviceAddress tempDeviceAddress;
        void printAddress(DeviceAddress deviceAddress);
        DallasTemperature *sensors;

    public:
        DS18B20(uint8_t oneWireBus);
        virtual ~DS18B20();
        virtual bool collectdata(JsonColl &collector) override;
        virtual void setupdevice(SubMenu &device_menu) override;
        virtual void onInputMessage(flyingCollection::SanCodedStr &data);
    };
}