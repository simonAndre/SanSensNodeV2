#pragma once
#include <SanSensNodeV2.h>
#define VOLTAGEPIN 33 //voltage measure (0 to 3.3v, use divided bridge)
#define ADC_a 0.00196164285990814 // voltage divided bridge : R1=990kH, R2=945kH
#define ADC_b 0.341586840689968

namespace SANSENSNODE_NAMESPACE
{
    class VoltageProbe : public DevicePlugin
    {
    private:
        const uint8_t nb_readings = 20;
        double ReadVoltage();
        double ReadVoltageOn3_3();

    public:
        VoltageProbe();
        virtual ~VoltageProbe();
        virtual bool collectdata(JsonColl &collector) override;
        virtual void firstSetup() override;
        virtual void setupdevice(SubMenu &device_menu) override;
        virtual void onInputMessage(flyingCollection::SanCodedStr &data) override;
    };
} // namespace SANSENSNODE_NAMESPACE