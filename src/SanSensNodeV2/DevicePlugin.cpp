#include "sansensnode/DevicePlugin.h"
#include "sansensnode/SanSensNodeV2.h"

namespace SANSENSNODE_NAMESPACE
{
    void DevicePlugin::hookSanSensInstance(SanSensNodeV2* instance)
    {
        _sansens_instance = instance;
    }

    SanSensNodeV2  DevicePlugin::getSanSensInstance(){
        return *_sansens_instance;
    }

} // namespace SANSENSNODE_NAMESPACE