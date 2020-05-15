#pragma once
#include <SanSensNodeV2.h>
#include <DHTesp.h> //use https://github.com/beegee-tokyo/DHTesp

#define DHT_WAITTIMEMS 0
namespace SANSENSNODE_NAMESPACE
{
    class DHT22 : public SensorPlugin
    {
    private:
        uint8_t _dhtpin;
        DHTesp dht;
        int _dhtWarmupTime ;

    public:
        DHT22(uint8_t dhtpin);
        virtual ~DHT22();
        virtual bool collectdata(JsonColl &collector) override;
        virtual void firstSetup() override;
        virtual void setupdevice(SubMenu &device_menu) override;
        virtual void onInputMessage(flyingCollection::SanCodedStr &data) override;
        void dht_setup();
    };
}