#include "../Configuration.h"
#include "../Namespace.h"
#include "DevicePlugin.h"
#include "SanSensNodeV2.h"

namespace SANSENSNODE_NAMESPACE
{
    DevicePlugin::DevicePlugin() {}
    DevicePlugin::~DevicePlugin() {}

    void DevicePlugin::hookSanSensInstance(SanSensNodeV2* instance)
    {
        _sansens_instance = instance;
    }

    SanSensNodeV2  DevicePlugin::getSanSensInstance(){
        return *_sansens_instance;
    }

    bool DevicePlugin::collectdata(JsonColl &collector){
        return false;
    }

    /**
         * @brief device setup actions and console menu entries
         * 
         * @param device_menu : "device" section submenu, to hook up additional menu entries 
         */
    void DevicePlugin::setupdevice(SubMenu &device_menu){}
    void DevicePlugin::onInputMessage(flyingCollection::SanCodedStr &data){}

} // namespace SANSENSNODE_NAMESPACE