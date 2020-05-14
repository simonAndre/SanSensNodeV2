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

    void DevicePlugin::setupdevice(SubMenu &device_menu) {}

    void DevicePlugin::onInputMessage(flyingCollection::SanCodedStr &data) {}

    inline const char *DevicePlugin::getDeviceName()
    {
        return _devicename.c_str();
    }

    void DevicePlugin::hookSanSensInstance(SanSensNodeV2 *instance) {}

    SanSensNodeV2* DevicePlugin::getSanSensInstance()
    {
        throw std::runtime_error(SANSENSNODE_NOTIMPL);
    }

} // namespace SANSENSNODE_NAMESPACE