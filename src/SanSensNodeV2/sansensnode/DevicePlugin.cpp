#include "../Configuration.h"
#include "../Namespace.h"
#include "../platform_logger.h"
#include "DevicePlugin.h"
#include <Arduino.h>

namespace SANSENSNODE_NAMESPACE
{

    SensorPlugin::SensorPlugin(const char *devicename) : _sensorname(std::string(devicename))
    {
    }

    SensorPlugin::~SensorPlugin() {}

    bool SensorPlugin::collectdata(JsonColl &collector)
    {
        throw std::runtime_error(SANSENSNODE_NOTIMPL);
    }
    void SensorPlugin::firstSetup()
    {
        enabled = true;
    }

    const char *SensorPlugin::enableMenuFunctionName()
    {
        std::string outstr(_sensorname.c_str());
        if (enabled)
            outstr.insert(0, "disable ");
        else
            outstr.insert(0, "enable ");
        return outstr.c_str();
    }

   

    void SensorPlugin::setupdevice(SubMenu &device_menu)
    {
     
    }

    void SensorPlugin::onInputMessage(flyingCollection::SanCodedStr &data) {}

    inline const char *SensorPlugin::getSensorName()
    {
        return _sensorname.c_str();
    }

    void SensorPlugin::hookSanSensInstance(SanSensNodeV2 *instance)
    {
        _sansens_instance = instance;
    }

    SanSensNodeV2 *SensorPlugin::getSanSensInstance()
    {
        return _sansens_instance;
    }

} // namespace SANSENSNODE_NAMESPACE