#include "../Configuration.h"
#include "../Namespace.h"
#include "../platform_logger.h"
#include "DevicePlugin.h"
#include <Arduino.h>

namespace SANSENSNODE_NAMESPACE
{

    DevicePlugin::DevicePlugin(const char *devicename) : _devicename(std::string(devicename))
    {
    }

    DevicePlugin::~DevicePlugin() {}

    bool DevicePlugin::collectdata(JsonColl &collector)
    {
        throw std::runtime_error(SANSENSNODE_NOTIMPL);
    }
    void DevicePlugin::firstSetup()
    {
        enabled = true;
    }

    const char *DevicePlugin::enableMenuFunctionName()
    {
        std::string outstr(_devicename.c_str());
        if (enabled)
            outstr.insert(0, "disable ");
        else
            outstr.insert(0, "enable ");
        return outstr.c_str();
    }

   

    void DevicePlugin::setupdevice(SubMenu &device_menu)
    {
     
    }

    void DevicePlugin::onInputMessage(flyingCollection::SanCodedStr &data) {}

    inline const char *DevicePlugin::getDeviceName()
    {
        return _devicename.c_str();
    }

    void DevicePlugin::hookSanSensInstance(SanSensNodeV2 *instance)
    {
        _sansens_instance = instance;
    }

    SanSensNodeV2 *DevicePlugin::getSanSensInstance()
    {
        return _sansens_instance;
    }

} // namespace SANSENSNODE_NAMESPACE