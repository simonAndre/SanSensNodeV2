#pragma once
// #include "SanSensNodeV2.h"
#include <flyingCollection.h>

namespace SANSENSNODE_NAMESPACE
{

    class SanSensNodeV2;
    /**
     * @brief base class for devices plugins
     * 
     */
    class DevicePlugin
    {
    private:
        SanSensNodeV2* _sansens_instance;

    public:
        DevicePlugin();
        virtual ~DevicePlugin();

        virtual bool collectdata(JsonColl &collector);

        /**
         * @brief device setup actions and console menu entries
         * 
         * @param device_menu : "device" section submenu, to hook up additional menu entries 
         */
        virtual void setupdevice(SubMenu &device_menu);
        virtual void onInputMessage(flyingCollection::SanCodedStr &data);
        void hookSanSensInstance(SanSensNodeV2 *instance);
        SanSensNodeV2 getSanSensInstance();
    };
} // namespace SANSENSNODE_NAMESPACE