#pragma once
#include <SanSensNodeV2.h>
#define LED1PIN 16
#define LED2PIN 17
namespace SANSENSNODE_NAMESPACE
{
    class TestPlugin : public DevicePlugin
    {
    private:
        bool led1On();
        bool led2On();

    public:
        TestPlugin();
        virtual ~TestPlugin();
        virtual bool collectdata(JsonColl &collector) override;
        virtual void firstSetup() override;
        virtual void setupdevice(SubMenu &device_menu) override;
        virtual void onInputMessage(flyingCollection::SanCodedStr &data) override;
    };
} // namespace SANSENSNODE_NAMESPACE